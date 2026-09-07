#include "cad2sim/engineering/engineering_model.hpp"
#include "cad2sim/engineering/engineering_model_serializer.hpp"
#include "cad2sim/engineering/region_identification.hpp"
#include "cad2sim/kernel/geometry_kernel.hpp"
#include "cad2sim/mesh/mesh_quality.hpp"
#include "cad2sim/mesh/surface_mesher.hpp"

#include <Standard_Version.hxx>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <vector>

#ifndef CAD2SIM_BENCHMARK_SOURCE_DIR
#error "CAD2SIM_BENCHMARK_SOURCE_DIR must be defined by CMake"
#endif

#ifndef CAD2SIM_BENCHMARK_BUILD_TYPE
#define CAD2SIM_BENCHMARK_BUILD_TYPE "unspecified"
#endif

namespace {

using Clock = std::chrono::steady_clock;

constexpr const char* kBenchmarkVersion = "1.0";
constexpr std::size_t kWarmupRuns = 1;
constexpr std::size_t kMeasuredRuns = 5;

struct TimingSample {
    double step_import_validation_ms = 0.0;
    double surface_meshing_ms = 0.0;
    double mesh_quality_ms = 0.0;
    double region_identification_ms = 0.0;
    double engineering_model_validation_ms = 0.0;
    double serialization_ms = 0.0;
    double total_preprocessing_ms = 0.0;
};

struct TimingStatistics {
    double min = 0.0;
    double median = 0.0;
    double mean = 0.0;
    double max = 0.0;
};

struct TimingReport {
    TimingStatistics step_import_validation;
    TimingStatistics surface_meshing;
    TimingStatistics mesh_quality;
    TimingStatistics region_identification;
    TimingStatistics engineering_model_validation;
    TimingStatistics serialization;
    TimingStatistics total_preprocessing;
};

struct TopologyMetrics {
    std::size_t solids = 0;
    std::size_t shells = 0;
    std::size_t faces = 0;
    std::size_t wires = 0;
    std::size_t edges = 0;
    std::size_t vertices = 0;
};

struct MeshMetrics {
    std::size_t mesh_faces = 0;
    std::size_t mesh_nodes = 0;
    std::size_t mesh_triangles = 0;
};

struct EngineeringMetrics {
    std::size_t regions = 0;
    std::size_t serialized_bytes = 0;
};

struct CaseReport {
    std::string id;
    std::string input_model;
    bool success = false;
    std::string diagnostic;

    TopologyMetrics topology;
    MeshMetrics mesh;
    std::size_t quality_elements = 0;
    EngineeringMetrics engineering;
    TimingReport timings;
};

struct BenchmarkMetadata {
    std::string benchmark_version;
    std::string timestamp_utc;
    std::string git_commit;
    std::string hardware;
    std::string operating_system;
    std::string compiler;
    std::string build_type;
    std::string opencascade_version;
};

struct BenchmarkConfig {
    double linear_deflection = 0.1;
    double angular_deflection = 0.5;
    std::size_t warmup_runs = kWarmupRuns;
    std::size_t measured_runs = kMeasuredRuns;
    std::string memory_method;
};

struct BenchmarkResult {
    BenchmarkMetadata metadata;
    BenchmarkConfig config;
    std::vector<CaseReport> cases;
    bool success = false;
    bool peak_memory_available = false;
    std::size_t peak_rss_kib = 0;
};

std::string json_escape(const std::string& input)
{
    std::ostringstream out;

    for (const char c : input) {
        switch (c) {
        case '\\':
            out << "\\\\";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            out << c;
            break;
        }
    }

    return out.str();
}

std::string utc_timestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
    gmtime_r(&time, &tm);

    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

std::string command_output(const std::string& command)
{
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "unavailable";
    }

    char buffer[256];
    std::string result;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    const int status = pclose(pipe);
    if (status != 0) {
        return "unavailable";
    }

    while (!result.empty() &&
           (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }

    return result.empty() ? "unavailable" : result;
}

std::string git_commit()
{
    const std::string source_dir = CAD2SIM_BENCHMARK_SOURCE_DIR;
    const std::string command =
        "git -C \"" + source_dir + "\" rev-parse HEAD 2>/dev/null";

    return command_output(command);
}

std::string hardware_model()
{
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;

    while (std::getline(cpuinfo, line)) {
        const std::string prefix = "model name";
        if (line.rfind(prefix, 0) == 0) {
            const std::size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string value = line.substr(colon + 1);

                while (!value.empty() && value.front() == ' ') {
                    value.erase(value.begin());
                }

                return value;
            }
        }
    }

    return "unavailable";
}

std::string operating_system()
{
    struct utsname info{};

    if (uname(&info) != 0) {
        return "unavailable";
    }

    std::ostringstream out;
    out << info.sysname << " " << info.release << " " << info.machine;
    return out.str();
}

std::string compiler_version()
{
#if defined(__clang__)
    return std::string("Clang ") + __clang_version__;
#elif defined(__GNUC__)
    return std::string("GCC ") + __VERSION__;
#else
    return "unknown";
#endif
}

bool read_peak_rss(std::size_t& rss_kib)
{
    struct rusage usage{};

    if (getrusage(RUSAGE_SELF, &usage) != 0) {
        return false;
    }

    if (usage.ru_maxrss <= 0) {
        return false;
    }

    rss_kib = static_cast<std::size_t>(usage.ru_maxrss);
    return true;
}

std::string format_ms(double value)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << value;
    return out.str();
}

TimingStatistics statistics(std::vector<double> values)
{
    std::sort(values.begin(), values.end());

    TimingStatistics result;

    result.min = values.front();
    result.max = values.back();

    result.mean =
        std::accumulate(values.begin(), values.end(), 0.0) /
        static_cast<double>(values.size());

    if (values.size() % 2 == 0) {
        const std::size_t middle = values.size() / 2;
        result.median = (values[middle - 1] + values[middle]) / 2.0;
    } else {
        result.median = values[values.size() / 2];
    }

    return result;
}

TimingReport make_timing_report(const std::vector<TimingSample>& samples)
{
    std::vector<double> step_import;
    std::vector<double> meshing;
    std::vector<double> quality;
    std::vector<double> regions;
    std::vector<double> validation;
    std::vector<double> serialization;
    std::vector<double> total;

    for (const auto& sample : samples) {
        step_import.push_back(sample.step_import_validation_ms);
        meshing.push_back(sample.surface_meshing_ms);
        quality.push_back(sample.mesh_quality_ms);
        regions.push_back(sample.region_identification_ms);
        validation.push_back(sample.engineering_model_validation_ms);
        serialization.push_back(sample.serialization_ms);
        total.push_back(sample.total_preprocessing_ms);
    }

    return {
        statistics(step_import),
        statistics(meshing),
        statistics(quality),
        statistics(regions),
        statistics(validation),
        statistics(serialization),
        statistics(total)
    };
}

cad2sim::engineering::EngineeringModel make_engineering_model(
    const std::vector<cad2sim::engineering::EngineeringRegion>& regions)
{
    cad2sim::engineering::EngineeringModel model;
    model.regions = regions;

    if (!regions.empty()) {
        model.material_assignments.push_back({0, "steel"});

        std::vector<std::size_t> all_regions;
        all_regions.reserve(regions.size());

        for (const auto& region : regions) {
            all_regions.push_back(region.index);
        }

        model.analysis_entities.push_back(
            {"structural", all_regions}
        );

        model.boundary_condition_references.push_back(
            {"fixed_support", {regions.front().index}}
        );
    }

    model.metadata = {
        {"preprocessing_stage", "performance_benchmark"},
        {"source_format", "STEP"}
    };

    return model;
}

bool same_metrics(
    const CaseReport& expected,
    const cad2sim::mesh::SurfaceMesh& mesh,
    const cad2sim::mesh::MeshQualityResult& quality,
    const cad2sim::engineering::RegionIdentificationResult& regions,
    const cad2sim::engineering::EngineeringModelSerializationResult& serialized)
{
    MeshMetrics mesh_metrics{};

    mesh_metrics.mesh_faces = mesh.faces.size();

    for (const auto& face : mesh.faces) {
        mesh_metrics.mesh_nodes += face.nodes.size();
        mesh_metrics.mesh_triangles += face.triangles.size();
    }

    return
        expected.mesh.mesh_faces == mesh_metrics.mesh_faces &&
        expected.mesh.mesh_nodes == mesh_metrics.mesh_nodes &&
        expected.mesh.mesh_triangles == mesh_metrics.mesh_triangles &&
        expected.quality_elements == quality.element_count &&
        expected.engineering.regions == regions.regions.size() &&
        expected.engineering.serialized_bytes == serialized.data.size();
}

bool run_pipeline(
    const std::filesystem::path& step_path,
    const cad2sim::mesh::SurfaceMeshParameters& parameters,
    CaseReport& report,
    bool capture_metrics)
{
    cad2sim::kernel::GeometryKernel kernel;
    cad2sim::mesh::SurfaceMesher mesher;
    cad2sim::mesh::MeshQualityEvaluator quality_evaluator;
    cad2sim::engineering::RegionIdentifier region_identifier;
    cad2sim::engineering::EngineeringModelValidator model_validator;
    cad2sim::engineering::EngineeringModelSerializer serializer;

    const auto total_start = Clock::now();

    const auto import_start = Clock::now();
    const auto loaded =
        kernel.load_validated_shape(step_path.string());
    const auto import_end = Clock::now();

    report.timings.step_import_validation.min =
        std::chrono::duration<double, std::milli>(
            import_end - import_start).count();

    if (!loaded.success) {
        report.diagnostic =
            "load_validated_shape failed: " + loaded.diagnostic;
        return false;
    }

    const auto mesh_start = Clock::now();
    const auto mesh_result =
        mesher.mesh(loaded.shape, parameters);
    const auto mesh_end = Clock::now();

    report.timings.surface_meshing.min =
        std::chrono::duration<double, std::milli>(
            mesh_end - mesh_start).count();

    if (!mesh_result.success) {
        report.diagnostic =
            "surface meshing failed: " + mesh_result.diagnostic;
        return false;
    }

    const auto quality_start = Clock::now();
    const auto quality_result =
        quality_evaluator.evaluate(mesh_result.mesh);
    const auto quality_end = Clock::now();

    report.timings.mesh_quality.min =
        std::chrono::duration<double, std::milli>(
            quality_end - quality_start).count();

    if (!quality_result.success) {
        report.diagnostic =
            "mesh quality evaluation failed: " +
            quality_result.diagnostic;
        return false;
    }

    const auto region_start = Clock::now();
    const auto region_result =
        region_identifier.identify(loaded.shape);
    const auto region_end = Clock::now();

    report.timings.region_identification.min =
        std::chrono::duration<double, std::milli>(
            region_end - region_start).count();

    if (!region_result.success) {
        report.diagnostic =
            "region identification failed: " +
            region_result.diagnostic;
        return false;
    }

    const auto model =
        make_engineering_model(region_result.regions);

    const auto validation_start = Clock::now();
    const auto model_validation =
        model_validator.validate(model);
    const auto validation_end = Clock::now();

    report.timings.engineering_model_validation.min =
        std::chrono::duration<double, std::milli>(
            validation_end - validation_start).count();

    if (!model_validation.success) {
        report.diagnostic =
            "engineering model validation failed: " +
            model_validation.diagnostic;
        return false;
    }

    const auto serialization_start = Clock::now();
    const auto serialized = serializer.serialize(model);
    const auto serialization_end = Clock::now();

    report.timings.serialization.min =
        std::chrono::duration<double, std::milli>(
            serialization_end - serialization_start).count();

    if (!serialized.success) {
        report.diagnostic =
            "engineering model serialization failed: " +
            serialized.diagnostic;
        return false;
    }

    const auto total_end = Clock::now();

    report.timings.total_preprocessing.min =
        std::chrono::duration<double, std::milli>(
            total_end - total_start).count();

    if (capture_metrics) {
        const auto topology =
            kernel.inspect_topology(step_path.string());

        report.topology.solids = topology.solids;
        report.topology.shells = topology.shells;
        report.topology.faces = topology.faces;
        report.topology.wires = topology.wires;
        report.topology.edges = topology.edges;
        report.topology.vertices = topology.vertices;

        report.mesh.mesh_faces = mesh_result.mesh.faces.size();

        for (const auto& face : mesh_result.mesh.faces) {
            report.mesh.mesh_nodes += face.nodes.size();
            report.mesh.mesh_triangles += face.triangles.size();
        }

        report.quality_elements = quality_result.element_count;
        report.engineering.regions = region_result.regions.size();
        report.engineering.serialized_bytes = serialized.data.size();
    } else {
        if (!same_metrics(
                report,
                mesh_result.mesh,
                quality_result,
                region_result,
                serialized)) {
            report.diagnostic =
                "structural metrics changed between benchmark runs";
            return false;
        }
    }

    return true;
}

void print_statistics(
    const std::string& name,
    const TimingStatistics& stats)
{
    std::cout
        << "    " << std::left << std::setw(32) << name
        << "min=" << std::right << std::setw(10)
        << format_ms(stats.min)
        << " ms  median=" << std::setw(10)
        << format_ms(stats.median)
        << " ms  mean=" << std::setw(10)
        << format_ms(stats.mean)
        << " ms  max=" << std::setw(10)
        << format_ms(stats.max)
        << " ms"
        << '\n';
}

void write_statistics_json(
    std::ostream& out,
    const TimingStatistics& stats)
{
    out << "{"
        << "\"min_ms\":" << stats.min << ","
        << "\"median_ms\":" << stats.median << ","
        << "\"mean_ms\":" << stats.mean << ","
        << "\"max_ms\":" << stats.max
        << "}";
}

void write_json(
    const BenchmarkResult& result,
    const std::filesystem::path& output_path)
{
    std::ofstream out(output_path);

    if (!out) {
        throw std::runtime_error(
            "cannot open JSON output: " + output_path.string());
    }

    out << std::fixed << std::setprecision(6);

    out << "{\n";

    out << "  \"metadata\": {\n";
    out << "    \"benchmark_version\": \""
        << json_escape(result.metadata.benchmark_version) << "\",\n";
    out << "    \"timestamp_utc\": \""
        << json_escape(result.metadata.timestamp_utc) << "\",\n";
    out << "    \"git_commit\": \""
        << json_escape(result.metadata.git_commit) << "\",\n";
    out << "    \"hardware\": \""
        << json_escape(result.metadata.hardware) << "\",\n";
    out << "    \"operating_system\": \""
        << json_escape(result.metadata.operating_system) << "\",\n";
    out << "    \"compiler\": \""
        << json_escape(result.metadata.compiler) << "\",\n";
    out << "    \"build_type\": \""
        << json_escape(result.metadata.build_type) << "\",\n";
    out << "    \"opencascade_version\": \""
        << json_escape(result.metadata.opencascade_version) << "\"\n";
    out << "  },\n";

    out << "  \"configuration\": {\n";
    out << "    \"linear_deflection\": "
        << result.config.linear_deflection << ",\n";
    out << "    \"angular_deflection\": "
        << result.config.angular_deflection << ",\n";
    out << "    \"warmup_runs\": "
        << result.config.warmup_runs << ",\n";
    out << "    \"measured_runs\": "
        << result.config.measured_runs << ",\n";
    out << "    \"memory_method\": \""
        << json_escape(result.config.memory_method) << "\"\n";
    out << "  },\n";

    out << "  \"cases\": [\n";

    for (std::size_t i = 0; i < result.cases.size(); ++i) {
        const auto& c = result.cases[i];

        out << "    {\n";
        out << "      \"id\": \"" << json_escape(c.id) << "\",\n";
        out << "      \"input_model\": \""
            << json_escape(c.input_model) << "\",\n";
        out << "      \"status\": \""
            << (c.success ? "PASS" : "FAIL") << "\"";

        if (!c.diagnostic.empty()) {
            out << ",\n      \"diagnostic\": \""
                << json_escape(c.diagnostic) << "\"";
        }

        out << ",\n      \"topology\": {\n";
        out << "        \"solids\": " << c.topology.solids << ",\n";
        out << "        \"shells\": " << c.topology.shells << ",\n";
        out << "        \"faces\": " << c.topology.faces << ",\n";
        out << "        \"wires\": " << c.topology.wires << ",\n";
        out << "        \"edges\": " << c.topology.edges << ",\n";
        out << "        \"vertices\": " << c.topology.vertices << "\n";
        out << "      },\n";

        out << "      \"mesh\": {\n";
        out << "        \"mesh_faces\": " << c.mesh.mesh_faces << ",\n";
        out << "        \"mesh_nodes\": " << c.mesh.mesh_nodes << ",\n";
        out << "        \"mesh_triangles\": " << c.mesh.mesh_triangles << "\n";
        out << "      },\n";

        out << "      \"quality\": {\n";
        out << "        \"quality_elements\": "
            << c.quality_elements << "\n";
        out << "      },\n";

        out << "      \"engineering\": {\n";
        out << "        \"regions\": "
            << c.engineering.regions << ",\n";
        out << "        \"serialized_bytes\": "
            << c.engineering.serialized_bytes << "\n";
        out << "      },\n";

        out << "      \"timings_ms\": {\n";

        out << "        \"step_import_validation\": ";
        write_statistics_json(out, c.timings.step_import_validation);
        out << ",\n";

        out << "        \"surface_meshing\": ";
        write_statistics_json(out, c.timings.surface_meshing);
        out << ",\n";

        out << "        \"mesh_quality\": ";
        write_statistics_json(out, c.timings.mesh_quality);
        out << ",\n";

        out << "        \"region_identification\": ";
        write_statistics_json(out, c.timings.region_identification);
        out << ",\n";

        out << "        \"engineering_model_validation\": ";
        write_statistics_json(
            out,
            c.timings.engineering_model_validation);
        out << ",\n";

        out << "        \"serialization\": ";
        write_statistics_json(out, c.timings.serialization);
        out << ",\n";

        out << "        \"total_preprocessing\": ";
        write_statistics_json(out, c.timings.total_preprocessing);
        out << "\n";

        out << "      }\n";
        out << "    }";

        if (i + 1 != result.cases.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ],\n";

    out << "  \"memory\": {\n";
    out << "    \"available\": "
        << (result.peak_memory_available ? "true" : "false") << ",\n";
    out << "    \"scope\": \"benchmark_process\",\n";
    out << "    \"method\": \""
        << json_escape(result.config.memory_method) << "\"";

    if (result.peak_memory_available) {
        out << ",\n    \"peak_rss_kib\": "
            << result.peak_rss_kib;
    }

    out << "\n  },\n";

    out << "  \"result\": \""
        << (result.success ? "PASS" : "FAIL")
        << "\"\n";

    out << "}\n";
}

bool parse_output_argument(
    int argc,
    char** argv,
    std::filesystem::path& output_path)
{
    output_path =
        std::filesystem::path("benchmark_results.json");

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];

        if (argument == "--output-json") {
            if (i + 1 >= argc) {
                std::cerr
                    << "ERROR: --output-json requires a path\n";
                return false;
            }

            output_path = argv[++i];
        } else if (argument == "--help") {
            std::cout
                << "Usage: cad2sim_preprocessing_benchmark "
                   "[--output-json PATH]\n";
            return false;
        } else {
            std::cerr
                << "ERROR: unknown argument: "
                << argument << '\n';
            return false;
        }
    }

    return true;
}

} // namespace

int main(int argc, char** argv)
{
    std::filesystem::path output_path;

    if (!parse_output_argument(argc, argv, output_path)) {
        return 2;
    }

    const std::filesystem::path source_root =
        CAD2SIM_BENCHMARK_SOURCE_DIR;

    const std::filesystem::path fixture_root =
        source_root / "tests" / "fixtures" / "step";

    struct BenchmarkCase {
        const char* id;
        const char* filename;
    };

    const std::vector<BenchmarkCase> cases = {
        {"screw", "screw.step"},
        {"hole-001", "hole-001.step"},
        {"pocket-001", "pocket-001.step"},
        {"fillet-001", "fillet-001.step"},
        {"chamfer-001", "chamfer-001.step"}
    };

    BenchmarkResult result;

    result.metadata.benchmark_version = kBenchmarkVersion;
    result.metadata.timestamp_utc = utc_timestamp();
    result.metadata.git_commit = git_commit();
    result.metadata.hardware = hardware_model();
    result.metadata.operating_system = operating_system();
    result.metadata.compiler = compiler_version();
    result.metadata.build_type = CAD2SIM_BENCHMARK_BUILD_TYPE;
    result.metadata.opencascade_version = OCC_VERSION_STRING;

    result.config.memory_method =
        "Linux getrusage(RUSAGE_SELF).ru_maxrss; "
        "process-level high-water mark, not per-case";

    std::cout
        << "============================================================\n"
        << "CAD2SIM REQ-019 PREPROCESSING BENCHMARK\n"
        << "============================================================\n";

    std::cout
        << "Benchmark version : "
        << result.metadata.benchmark_version << '\n'
        << "Git commit        : "
        << result.metadata.git_commit << '\n'
        << "Hardware          : "
        << result.metadata.hardware << '\n'
        << "OS                : "
        << result.metadata.operating_system << '\n'
        << "Compiler          : "
        << result.metadata.compiler << '\n'
        << "Build type        : "
        << result.metadata.build_type << '\n'
        << "OpenCASCADE       : "
        << result.metadata.opencascade_version << '\n'
        << "Linear deflection : "
        << result.config.linear_deflection << '\n'
        << "Angular deflection: "
        << result.config.angular_deflection << '\n'
        << "Warm-up runs      : "
        << result.config.warmup_runs << '\n'
        << "Measured runs     : "
        << result.config.measured_runs << '\n'
        << '\n';

    bool all_success = true;

    for (const auto& benchmark_case : cases) {
        const std::filesystem::path step_path =
            fixture_root / benchmark_case.filename;

        CaseReport report;
        report.id = benchmark_case.id;
        report.input_model = step_path.string();

        std::cout
            << "------------------------------------------------------------\n"
            << "CASE: " << report.id << '\n'
            << "INPUT: " << report.input_model << '\n';

        if (!std::filesystem::exists(step_path)) {
            report.diagnostic =
                "input fixture does not exist";

            std::cerr
                << "[FAIL] " << report.diagnostic << '\n';

            report.success = false;
            result.cases.push_back(report);
            all_success = false;
            continue;
        }

        cad2sim::mesh::SurfaceMeshParameters parameters;
        parameters.linear_deflection =
            result.config.linear_deflection;
        parameters.angular_deflection =
            result.config.angular_deflection;

        std::cout << "Warm-up run...\n";

        CaseReport warmup_report = report;

        if (!run_pipeline(
                step_path,
                parameters,
                warmup_report,
                true)) {
            report.diagnostic =
                "warm-up failed: " + warmup_report.diagnostic;

            std::cerr
                << "[FAIL] " << report.diagnostic << '\n';

            report.success = false;
            result.cases.push_back(report);
            all_success = false;
            continue;
        }

        std::vector<TimingSample> samples;
        samples.reserve(kMeasuredRuns);

        report.topology = warmup_report.topology;
        report.mesh = warmup_report.mesh;
        report.quality_elements =
            warmup_report.quality_elements;
        report.engineering =
            warmup_report.engineering;

        for (std::size_t run = 0;
             run < kMeasuredRuns;
             ++run) {
            CaseReport run_report = report;

            std::cout
                << "Measured run "
                << (run + 1)
                << "/"
                << kMeasuredRuns
                << "...\n";

            if (!run_pipeline(
                    step_path,
                    parameters,
                    run_report,
                    false)) {
                report.diagnostic =
                    "measured run " +
                    std::to_string(run + 1) +
                    " failed: " +
                    run_report.diagnostic;

                std::cerr
                    << "[FAIL] " << report.diagnostic << '\n';

                report.success = false;
                all_success = false;
                break;
            }

            TimingSample sample;

            sample.step_import_validation_ms =
                run_report.timings.step_import_validation.min;

            sample.surface_meshing_ms =
                run_report.timings.surface_meshing.min;

            sample.mesh_quality_ms =
                run_report.timings.mesh_quality.min;

            sample.region_identification_ms =
                run_report.timings.region_identification.min;

            sample.engineering_model_validation_ms =
                run_report.timings.engineering_model_validation.min;

            sample.serialization_ms =
                run_report.timings.serialization.min;

            sample.total_preprocessing_ms =
                run_report.timings.total_preprocessing.min;

            samples.push_back(sample);
        }

        if (samples.size() == kMeasuredRuns) {
            report.timings =
                make_timing_report(samples);
            report.success = true;

            std::cout << "[PASS] " << report.id << '\n';

            std::cout
                << "  Topology: solids="
                << report.topology.solids
                << ", shells="
                << report.topology.shells
                << ", faces="
                << report.topology.faces
                << ", wires="
                << report.topology.wires
                << ", edges="
                << report.topology.edges
                << ", vertices="
                << report.topology.vertices
                << '\n';

            std::cout
                << "  Mesh: faces="
                << report.mesh.mesh_faces
                << ", nodes="
                << report.mesh.mesh_nodes
                << ", triangles="
                << report.mesh.mesh_triangles
                << '\n';

            std::cout
                << "  Quality elements: "
                << report.quality_elements
                << '\n';

            std::cout
                << "  Engineering: regions="
                << report.engineering.regions
                << ", serialized_bytes="
                << report.engineering.serialized_bytes
                << '\n';

            print_statistics(
                "step_import_validation",
                report.timings.step_import_validation);

            print_statistics(
                "surface_meshing",
                report.timings.surface_meshing);

            print_statistics(
                "mesh_quality",
                report.timings.mesh_quality);

            print_statistics(
                "region_identification",
                report.timings.region_identification);

            print_statistics(
                "engineering_model_validation",
                report.timings.engineering_model_validation);

            print_statistics(
                "serialization",
                report.timings.serialization);

            print_statistics(
                "total_preprocessing",
                report.timings.total_preprocessing);
        }

        result.cases.push_back(report);
    }

    std::size_t peak_rss_kib = 0;

    if (read_peak_rss(peak_rss_kib)) {
        result.peak_memory_available = true;
        result.peak_rss_kib = peak_rss_kib;

        std::cout
            << '\n'
            << "Peak RSS (process-level): "
            << peak_rss_kib
            << " KiB\n";
    } else {
        std::cout
            << '\n'
            << "Peak RSS: unavailable\n";
    }

    result.success =
        all_success &&
        result.cases.size() == cases.size();

    try {
        const std::filesystem::path parent =
            output_path.parent_path();

        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }

        write_json(result, output_path);

        std::cout
            << "JSON report: "
            << output_path
            << '\n';
    } catch (const std::exception& error) {
        std::cerr
            << "ERROR: failed to write JSON report: "
            << error.what()
            << '\n';

        return 3;
    }

    std::cout
        << "============================================================\n"
        << "RESULT: "
        << (result.success ? "PASS" : "FAIL")
        << '\n'
        << "============================================================\n";

    return result.success ? 0 : 1;
}
