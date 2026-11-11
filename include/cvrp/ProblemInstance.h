#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>

namespace cvrp {

struct Point {
    double x{0.0};
    double y{0.0};
};

class ProblemInstance {
public:
    ProblemInstance() = default;

    ProblemInstance(std::string name,
                    int dimension,
                    int capacity,
                    int vehicleCount,
                    int depotIndex,
                    std::vector<Point> coords,
                    std::vector<int> demands,
                    std::vector<int> permutation);

    [[nodiscard]] const std::string& getName() const noexcept { return name; }
    [[nodiscard]] int getDimension() const noexcept { return dimension; }
    [[nodiscard]] int getCapacity() const noexcept { return capacity; }
    [[nodiscard]] int getVehicleCount() const noexcept { return vehicleCount; }
    [[nodiscard]] int getDepotIndex() const noexcept { return depotIndex; }
    [[nodiscard]] int getClientCount() const noexcept { return static_cast<int>(clientPermutation.size()); }

    [[nodiscard]] const std::vector<Point>& getCoords() const noexcept { return coords; }
    [[nodiscard]] const std::vector<int>& getDemands() const noexcept { return demands; }
    [[nodiscard]] const std::vector<int>& getClientPermutation() const noexcept { return clientPermutation; }

    [[nodiscard]] int getDemand(int nodeId) const;
    [[nodiscard]] int getPermutedClient(int index) const;

    [[nodiscard]] inline int getDistance(int from, int to) const noexcept {
        return distanceMatrix[from * dimension + to];
    }

private:
    void computeDistanceMatrix();

    std::string name;
    int dimension{0};
    int capacity{0};
    int vehicleCount{0};
    int depotIndex{0};

    std::vector<Point> coords;
    std::vector<int> demands;
    std::vector<int> clientPermutation;
    std::vector<int> distanceMatrix; // Flattened 1D matrix for optimal CPU L1/L2 cache locality
};

} // namespace cvrp
