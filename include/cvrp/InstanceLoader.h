#pragma once

#include "cvrp/ProblemInstance.h"
#include <string>
#include <optional>
#include <filesystem>

namespace cvrp {

class InstanceLoader {
public:
    InstanceLoader() = default;

    [[nodiscard]] static std::optional<ProblemInstance> loadFromFile(const std::filesystem::path& filePath);
    [[nodiscard]] static int parseVehicleCount(const std::string& fileName);
};

} // namespace cvrp
