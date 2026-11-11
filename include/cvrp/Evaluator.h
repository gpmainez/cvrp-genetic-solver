#pragma once

#include "cvrp/ProblemInstance.h"
#include "cvrp/Individual.h"
#include <vector>
#include <string>

namespace cvrp {

struct SubRoute {
    std::vector<int> nodes; // Includes depot start, clients, depot return
    int totalDemand{0};
    double distance{0.0};
};

struct VehicleRoute {
    int vehicleId{0};
    std::vector<SubRoute> trips;
    double totalDistance{0.0};
    int totalDemand{0};
};

class Evaluator {
public:
    explicit Evaluator(const ProblemInstance& instance);

    [[nodiscard]] double evaluate(const std::vector<int>& genotype) const;
    [[nodiscard]] double evaluate(const Individual& individual) const;

    // Evaluates entire population in parallel using standard C++ threads
    void evaluatePopulation(std::vector<Individual>& population, int threadCount = 0) const;

    [[nodiscard]] std::vector<VehicleRoute> getDetailedRoutes(const Individual& individual) const;

    [[nodiscard]] const ProblemInstance& getInstance() const noexcept { return instance; }

private:
    [[nodiscard]] double computeGroupCost(int groupId, const std::vector<int>& genotype) const;

    const ProblemInstance& instance;
};

} // namespace cvrp
