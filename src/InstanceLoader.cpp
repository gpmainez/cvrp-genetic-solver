#include "cvrp/InstanceLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

namespace cvrp {

int InstanceLoader::parseVehicleCount(const std::string& fileName) {
    // Looks for "-k<num>" or "k<num>" in benchmark filename (e.g., A-n33-k5.lcvrp -> 5)
    std::regex kPattern(R"([kK](\d+))");
    std::smatch match;
    if (std::regex_search(fileName, match, kPattern) && match.size() > 1) {
        try {
            return std::stoi(match[1].str());
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

std::optional<ProblemInstance> InstanceLoader::loadFromFile(const std::filesystem::path& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::string instanceName = filePath.stem().string();
    int dimension = 0;
    int capacity = 0;
    int depotId = 1;
    int vehicleCount = parseVehicleCount(filePath.filename().string());

    std::vector<int> permutation;
    std::vector<Point> coords;
    std::vector<int> demands;

    std::string line;
    enum class Section { Header, Coords, Demands, Depot };
    Section currentSection = Section::Header;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line.find("NODE_COORD_SECTION") != std::string::npos) {
            currentSection = Section::Coords;
            if (dimension > 0) {
                coords.assign(dimension, Point{0.0, 0.0});
            }
            continue;
        }
        if (line.find("DEMAND_SECTION") != std::string::npos) {
            currentSection = Section::Demands;
            if (dimension > 0) {
                demands.assign(dimension, 0);
            }
            continue;
        }
        if (line.find("DEPOT_SECTION") != std::string::npos) {
            currentSection = Section::Depot;
            continue;
        }
        if (line.find("EOF") != std::string::npos) {
            break;
        }

        if (currentSection == Section::Header) {
            const auto colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);

                // Trim whitespace
                auto trim = [](std::string& s) {
                    s.erase(0, s.find_first_not_of(" \t\r\n"));
                    s.erase(s.find_last_not_of(" \t\r\n") + 1);
                };
                trim(key);
                trim(value);

                if (key == "NAME") {
                    instanceName = value;
                } else if (key == "DIMENSION") {
                    dimension = std::stoi(value);
                } else if (key == "CAPACITY") {
                    capacity = std::stoi(value);
                } else if (key == "PERMUTATION") {
                    std::istringstream pStream(value);
                    int nodeId = 0;
                    while (pStream >> nodeId) {
                        permutation.push_back(nodeId - 1); // 0-based node index
                    }
                }
            }
        } else if (currentSection == Section::Coords) {
            std::istringstream stream(line);
            int id = 0;
            double x = 0.0;
            double y = 0.0;
            if (stream >> id >> x >> y) {
                if (id >= 1 && id <= dimension) {
                    coords[id - 1] = Point{x, y};
                }
            }
        } else if (currentSection == Section::Demands) {
            std::istringstream stream(line);
            int id = 0;
            int demand = 0;
            if (stream >> id >> demand) {
                if (id >= 1 && id <= dimension) {
                    demands[id - 1] = demand;
                }
            }
        } else if (currentSection == Section::Depot) {
            std::istringstream stream(line);
            int val = 0;
            if (stream >> val && val > 0) {
                depotId = val;
            }
        }
    }

    if (dimension <= 0 || capacity <= 0 || coords.empty() || demands.empty()) {
        return std::nullopt;
    }

    // Default vehicle count if not derived from filename
    if (vehicleCount <= 0) {
        vehicleCount = std::max(1, dimension / 5);
    }

    // If permutation was not in header, default to sequential 1..N-1
    if (permutation.empty()) {
        permutation.reserve(dimension - 1);
        for (int i = 0; i < dimension; ++i) {
            if (i != (depotId - 1)) {
                permutation.push_back(i);
            }
        }
    }

    const int depotIndex = depotId - 1; // 0-based

    return ProblemInstance(instanceName,
                           dimension,
                           capacity,
                           vehicleCount,
                           depotIndex,
                           std::move(coords),
                           std::move(demands),
                           std::move(permutation));
}

} // namespace cvrp
