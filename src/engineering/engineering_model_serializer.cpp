#include "cad2sim/engineering/engineering_model_serializer.hpp"

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

namespace cad2sim::engineering {

namespace {

constexpr const char* kHeader = "CAD2SIM_ENGINEERING_MODEL 1";

bool parse_size(
    const std::string& token,
    std::size_t& value
) {
    if (token.empty()) {
        return false;
    }

    std::size_t parsed = 0;

    for (const char character : token) {
        if (character < '0' || character > '9') {
            return false;
        }

        const std::size_t digit =
            static_cast<std::size_t>(character - '0');

        if (parsed >
            (static_cast<std::size_t>(-1) - digit) / 10) {
            return false;
        }

        parsed = parsed * 10 + digit;
    }

    value = parsed;
    return true;
}

std::string validation_failure(
    const EngineeringModelValidationResult& result
) {
    if (!result.diagnostic.empty()) {
        return result.diagnostic;
    }

    return "Engineering model validation failed";
}

}  // namespace

EngineeringModelSerializationResult
EngineeringModelSerializer::serialize(
    const EngineeringModel& model
) const {
    EngineeringModelValidator validator;
    const auto validation = validator.validate(model);

    if (!validation.success) {
        return {
            false,
            {},
            validation_failure(validation)
        };
    }

    std::ostringstream output;

    output << kHeader << '\n';

    output << "REGIONS "
           << model.regions.size()
           << '\n';

    for (const auto& region : model.regions) {
        output << "REGION "
               << region.index
               << ' '
               << "FACE "
               << region.source.entity_index
               << '\n';
    }

    output << "MATERIAL_ASSIGNMENTS "
           << model.material_assignments.size()
           << '\n';

    for (const auto& assignment :
         model.material_assignments) {
        output << "MATERIAL "
               << assignment.region_index
               << ' '
               << assignment.material_id
               << '\n';
    }

    output << "ANALYSIS_ENTITIES "
           << model.analysis_entities.size()
           << '\n';

    for (const auto& entity :
         model.analysis_entities) {
        output << "ANALYSIS "
               << entity.id
               << ' '
               << entity.region_indices.size();

        for (const auto region_index :
             entity.region_indices) {
            output << ' ' << region_index;
        }

        output << '\n';
    }

    output << "BOUNDARY_CONDITION_REFERENCES "
           << model.boundary_condition_references.size()
           << '\n';

    for (const auto& reference :
         model.boundary_condition_references) {
        output << "BC "
               << reference.id
               << ' '
               << reference.region_indices.size();

        for (const auto region_index :
             reference.region_indices) {
            output << ' ' << region_index;
        }

        output << '\n';
    }

    output << "METADATA "
           << model.metadata.size()
           << '\n';

    for (const auto& metadata :
         model.metadata) {
        output << "META "
               << metadata.key
               << ' '
               << metadata.value
               << '\n';
    }

    output << "END\n";

    return {
        true,
        output.str(),
        {}
    };
}

EngineeringModelDeserializationResult
EngineeringModelSerializer::deserialize(
    const std::string& data
) const {
    EngineeringModel model;
    std::istringstream input(data);
    std::string line;

    if (!std::getline(input, line) ||
        line != kHeader) {
        return {
            false,
            {},
            "Engineering model deserialization failed: unsupported or missing version"
        };
    }

    auto read_count_section =
        [&](const char* expected,
            std::size_t& count) -> bool {
            if (!std::getline(input, line)) {
                return false;
            }

            std::istringstream section(line);
            std::string name;
            std::string count_token;

            section >> name >> count_token;

            if (!section ||
                name != expected ||
                !parse_size(count_token, count)) {
                return false;
            }

            std::string extra;
            if (section >> extra) {
                return false;
            }

            return true;
        };

    std::size_t count = 0;

    if (!read_count_section("REGIONS", count)) {
        return {
            false,
            {},
            "Engineering model deserialization failed: invalid REGIONS section"
        };
    }

    model.regions.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        if (!std::getline(input, line)) {
            return {
                false,
                {},
                "Engineering model deserialization failed: missing REGION record"
            };
        }

        std::istringstream record(line);
        std::string tag;
        std::string index_token;
        std::string entity_type;
        std::string entity_index_token;

        record >> tag
               >> index_token
               >> entity_type
               >> entity_index_token;

        std::size_t index = 0;
        std::size_t entity_index = 0;

        if (!record ||
            tag != "REGION" ||
            entity_type != "FACE" ||
            !parse_size(index_token, index) ||
            !parse_size(entity_index_token, entity_index)) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid REGION record"
            };
        }

        std::string extra;
        if (record >> extra) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid REGION record"
            };
        }

        model.regions.push_back({
            index,
            {RegionEntityType::Face, entity_index}
        });
    }

    if (!read_count_section(
            "MATERIAL_ASSIGNMENTS",
            count)) {
        return {
            false,
            {},
            "Engineering model deserialization failed: invalid MATERIAL_ASSIGNMENTS section"
        };
    }

    model.material_assignments.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        if (!std::getline(input, line)) {
            return {
                false,
                {},
                "Engineering model deserialization failed: missing MATERIAL record"
            };
        }

        std::istringstream record(line);
        std::string tag;
        std::string region_token;
        std::string material_id;

        record >> tag
               >> region_token
               >> material_id;

        std::size_t region_index = 0;

        if (!record ||
            tag != "MATERIAL" ||
            !parse_size(region_token, region_index) ||
            material_id.empty()) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid MATERIAL record"
            };
        }

        std::string extra;
        if (record >> extra) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid MATERIAL record"
            };
        }

        model.material_assignments.push_back({
            region_index,
            material_id
        });
    }

    if (!read_count_section(
            "ANALYSIS_ENTITIES",
            count)) {
        return {
            false,
            {},
            "Engineering model deserialization failed: invalid ANALYSIS_ENTITIES section"
        };
    }

    model.analysis_entities.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        if (!std::getline(input, line)) {
            return {
                false,
                {},
                "Engineering model deserialization failed: missing ANALYSIS record"
            };
        }

        std::istringstream record(line);
        std::string tag;
        std::string id;
        std::string region_count_token;
        std::size_t region_count = 0;

        record >> tag
               >> id
               >> region_count_token;

        if (!record ||
            tag != "ANALYSIS" ||
            id.empty() ||
            !parse_size(region_count_token, region_count)) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid ANALYSIS record"
            };
        }

        std::vector<std::size_t> region_indices;
        region_indices.reserve(region_count);

        for (std::size_t region = 0;
             region < region_count;
             ++region) {
            std::string region_token;
            std::size_t region_index = 0;

            record >> region_token;

            if (!record ||
                !parse_size(region_token, region_index)) {
                return {
                    false,
                    {},
                    "Engineering model deserialization failed: invalid ANALYSIS region reference"
                };
            }

            region_indices.push_back(region_index);
        }

        std::string extra;
        if (record >> extra) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid ANALYSIS record"
            };
        }

        model.analysis_entities.push_back({
            id,
            region_indices
        });
    }

    if (!read_count_section(
            "BOUNDARY_CONDITION_REFERENCES",
            count)) {
        return {
            false,
            {},
            "Engineering model deserialization failed: invalid BOUNDARY_CONDITION_REFERENCES section"
        };
    }

    model.boundary_condition_references.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        if (!std::getline(input, line)) {
            return {
                false,
                {},
                "Engineering model deserialization failed: missing BC record"
            };
        }

        std::istringstream record(line);
        std::string tag;
        std::string id;
        std::string region_count_token;
        std::size_t region_count = 0;

        record >> tag
               >> id
               >> region_count_token;

        if (!record ||
            tag != "BC" ||
            id.empty() ||
            !parse_size(region_count_token, region_count)) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid BC record"
            };
        }

        std::vector<std::size_t> region_indices;
        region_indices.reserve(region_count);

        for (std::size_t region = 0;
             region < region_count;
             ++region) {
            std::string region_token;
            std::size_t region_index = 0;

            record >> region_token;

            if (!record ||
                !parse_size(region_token, region_index)) {
                return {
                    false,
                    {},
                    "Engineering model deserialization failed: invalid BC region reference"
                };
            }

            region_indices.push_back(region_index);
        }

        std::string extra;
        if (record >> extra) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid BC record"
            };
        }

        model.boundary_condition_references.push_back({
            id,
            region_indices
        });
    }

    if (!read_count_section(
            "METADATA",
            count)) {
        return {
            false,
            {},
            "Engineering model deserialization failed: invalid METADATA section"
        };
    }

    model.metadata.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        if (!std::getline(input, line)) {
            return {
                false,
                {},
                "Engineering model deserialization failed: missing META record"
            };
        }

        std::istringstream record(line);
        std::string tag;
        std::string key;
        std::string value;

        record >> tag >> key >> value;

        if (!record ||
            tag != "META" ||
            key.empty()) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid META record"
            };
        }

        std::string extra;
        if (record >> extra) {
            return {
                false,
                {},
                "Engineering model deserialization failed: invalid META record"
            };
        }

        model.metadata.push_back({
            key,
            value
        });
    }

    if (!std::getline(input, line) ||
        line != "END") {
        return {
            false,
            {},
            "Engineering model deserialization failed: missing END marker"
        };
    }

    if (std::getline(input, line)) {
        return {
            false,
            {},
            "Engineering model deserialization failed: trailing data"
        };
    }

    EngineeringModelValidator validator;
    const auto validation = validator.validate(model);

    if (!validation.success) {
        return {
            false,
            {},
            validation_failure(validation)
        };
    }

    return {
        true,
        model,
        {}
    };
}

}  // namespace cad2sim::engineering
