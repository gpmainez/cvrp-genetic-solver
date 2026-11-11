#include "cvrp/ProblemInstance.h"
#include <cmath>
#include <stdexcept>

namespace cvrp {

ProblemInstance::ProblemInstance(std::string name,
                                 int dimension,
                                 int capacity,
                                 int vehicleCount,
                                 int depotIndex,
                                 std::vector<Point> coords,
                                 std::vector<int> demands,
                                 std::vector<int> permutation)
    : name(std::move(name)),
      dimension(dimension),
      capacity(capacity),
      vehicleCount(vehicleCount),
      depotIndex(depotIndex),
      coords(std::move(coords)),
      demands(std::move(demands)),
      clientPermutation(std::move(permutation)) {
    computeDistanceMatrix();
}

void ProblemInstance::computeDistanceMatrix() {
    if (dimension <= 0 || coords.size() != static_cast<size_t>(dimension)) {
        throw std::invalid_argument("Dimension does not match coordinate count.");
    }

    distanceMatrix.assign(dimension * dimension, 0);

    for (int i = 0; i < dimension; ++i) {
        for (int j = i + 1; j < dimension; ++j) {
            const double dx = coords[i].x - coords[j].x;
            const double dy = coords[i].y - coords[j].y;
            const int dist = static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy)));

            distanceMatrix[i * dimension + j] = dist;
            distanceMatrix[j * dimension + i] = dist;
        }
    }
}

int ProblemInstance::getDemand(int nodeId) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(demands.size())) {
        throw std::out_of_range("Node ID out of demands range: " + std::to_string(nodeId));
    }
    return demands[nodeId];
}

int ProblemInstance::getPermutedClient(int index) const {
    if (index < 0 || index >= static_cast<int>(clientPermutation.size())) {
        throw std::out_of_range("Permutation index out of range: " + std::to_string(index));
    }
    return clientPermutation[index];
}

} // namespace cvrp
