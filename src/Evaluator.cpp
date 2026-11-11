#include "cvrp/Evaluator.h"
#include <thread>
#include <future>
#include <algorithm>
#include <iostream>

namespace cvrp {

Evaluator::Evaluator(const ProblemInstance& instance) : instance(instance) {}

double Evaluator::computeGroupCost(int groupId, const std::vector<int>& genotype) const {
    const int depot = instance.getDepotIndex();
    const int capacity = instance.getCapacity();

    std::vector<int> groupClients;
    groupClients.reserve(genotype.size());

    for (size_t i = 0; i < genotype.size(); ++i) {
        if (genotype[i] == groupId) {
            groupClients.push_back(instance.getPermutedClient(static_cast<int>(i)));
        }
    }

    if (groupClients.empty()) {
        return 0.0;
    }

    double cost = 0.0;
    int currentLoad = 0;
    int lastNode = depot;

    for (int client : groupClients) {
        const int demand = instance.getDemand(client);

        if (demand > capacity) {
            return 1e9; // Infeasible demand penalty
        }

        if (currentLoad + demand > capacity) {
            // Vehicle must return to depot and start a new trip
            cost += instance.getDistance(lastNode, depot);
            cost += instance.getDistance(depot, client);
            currentLoad = demand;
            lastNode = client;
        } else {
            cost += instance.getDistance(lastNode, client);
            currentLoad += demand;
            lastNode = client;
        }
    }

    if (lastNode != depot) {
        cost += instance.getDistance(lastNode, depot);
    }

    return cost;
}

double Evaluator::evaluate(const std::vector<int>& genotype) const {
    if (genotype.size() != static_cast<size_t>(instance.getClientCount())) {
        return 1e9;
    }

    double totalCost = 0.0;
    const int vehicleCount = instance.getVehicleCount();

    for (int g = 0; g < vehicleCount; ++g) {
        totalCost += computeGroupCost(g, genotype);
    }

    return totalCost;
}

double Evaluator::evaluate(const Individual& individual) const {
    if (individual.isEvaluated()) {
        return individual.getFitness();
    }
    const double fit = evaluate(individual.getGenotype());
    individual.setFitness(fit);
    return fit;
}

void Evaluator::evaluatePopulation(std::vector<Individual>& population, int threadCount) const {
    if (population.empty()) return;

    if (threadCount <= 0) {
        threadCount = static_cast<int>(std::thread::hardware_concurrency());
        if (threadCount <= 0) threadCount = 4;
    }

    const size_t popSize = population.size();
    const size_t numThreads = std::min<size_t>(threadCount, popSize);

    if (numThreads <= 1) {
        for (auto& ind : population) {
            if (!ind.isEvaluated()) {
                ind.setFitness(evaluate(ind.getGenotype()));
            }
        }
        return;
    }

    std::vector<std::future<void>> futures;
    futures.reserve(numThreads);

    const size_t chunkSize = (popSize + numThreads - 1) / numThreads;

    for (size_t t = 0; t < numThreads; ++t) {
        const size_t start = t * chunkSize;
        const size_t end = std::min(start + chunkSize, popSize);

        if (start >= popSize) break;

        futures.push_back(std::async(std::launch::async, [this, &population, start, end]() {
            for (size_t i = start; i < end; ++i) {
                if (!population[i].isEvaluated()) {
                    const double fit = this->evaluate(population[i].getGenotype());
                    population[i].setFitness(fit);
                }
            }
        }));
    }

    for (auto& f : futures) {
        f.get();
    }
}

std::vector<VehicleRoute> Evaluator::getDetailedRoutes(const Individual& individual) const {
    std::vector<VehicleRoute> routes;
    const auto& genotype = individual.getGenotype();
    const int depot = instance.getDepotIndex();
    const int capacity = instance.getCapacity();
    const int vehicleCount = instance.getVehicleCount();

    routes.reserve(vehicleCount);

    for (int g = 0; g < vehicleCount; ++g) {
        VehicleRoute vRoute;
        vRoute.vehicleId = g;

        std::vector<int> groupClients;
        for (size_t i = 0; i < genotype.size(); ++i) {
            if (genotype[i] == g) {
                groupClients.push_back(instance.getPermutedClient(static_cast<int>(i)));
            }
        }

        if (groupClients.empty()) {
            routes.push_back(vRoute);
            continue;
        }

        SubRoute currentTrip;
        currentTrip.nodes.push_back(depot);
        int currentLoad = 0;
        int lastNode = depot;
        double groupTotalDistance = 0.0;

        for (int client : groupClients) {
            const int demand = instance.getDemand(client);

            if (currentLoad + demand > capacity) {
                // Return to depot, finish trip
                const double returnDist = instance.getDistance(lastNode, depot);
                currentTrip.distance += returnDist;
                currentTrip.nodes.push_back(depot);
                currentTrip.totalDemand = currentLoad;
                groupTotalDistance += currentTrip.distance;
                vRoute.trips.push_back(std::move(currentTrip));

                // Start new trip
                currentTrip = SubRoute{};
                currentTrip.nodes.push_back(depot);
                currentTrip.nodes.push_back(client);
                currentTrip.distance = instance.getDistance(depot, client);
                currentLoad = demand;
                lastNode = client;
            } else {
                currentTrip.nodes.push_back(client);
                currentTrip.distance += instance.getDistance(lastNode, client);
                currentLoad += demand;
                lastNode = client;
            }
        }

        if (lastNode != depot) {
            currentTrip.distance += instance.getDistance(lastNode, depot);
            currentTrip.nodes.push_back(depot);
        }
        currentTrip.totalDemand = currentLoad;
        groupTotalDistance += currentTrip.distance;
        vRoute.trips.push_back(std::move(currentTrip));

        vRoute.totalDistance = groupTotalDistance;
        for (const auto& trip : vRoute.trips) {
            vRoute.totalDemand += trip.totalDemand;
        }

        routes.push_back(std::move(vRoute));
    }

    return routes;
}

} // namespace cvrp
