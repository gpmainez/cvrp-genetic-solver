#include "cvrp/Individual.h"
#include <algorithm>

namespace cvrp {

Individual::Individual(std::vector<int> genotype)
    : genotype(std::move(genotype)), fitness(1e9), evaluated(false) {}

void Individual::mutate(double geneMutProb, int vehicleCount, std::mt19937_64& rng) {
    if (genotype.empty() || vehicleCount <= 1) {
        return;
    }

    std::uniform_real_distribution<double> probDist(0.0, 1.0);
    std::uniform_int_distribution<int> groupDist(0, vehicleCount - 1);

    bool modified = false;

    for (int& gene : genotype) {
        if (probDist(rng) < geneMutProb) {
            gene = groupDist(rng);
            modified = true;
        }
    }

    // Optional swap mutation (swap groups between two random clients)
    if (genotype.size() > 1 && probDist(rng) < (geneMutProb * 1.5)) {
        std::uniform_int_distribution<size_t> idxDist(0, genotype.size() - 1);
        const size_t i = idxDist(rng);
        const size_t j = idxDist(rng);
        if (i != j) {
            std::swap(genotype[i], genotype[j]);
            modified = true;
        }
    }

    if (modified) {
        invalidateFitness();
    }
}

std::pair<Individual, Individual> Individual::crossover(
    const Individual& parent1,
    const Individual& parent2,
    double crossProb,
    std::mt19937_64& rng
) {
    const size_t n = parent1.genotype.size();
    if (n == 0 || parent2.genotype.size() != n) {
        return {parent1, parent2};
    }

    std::uniform_real_distribution<double> probDist(0.0, 1.0);

    if (probDist(rng) >= crossProb) {
        return {parent1, parent2};
    }

    std::uniform_int_distribution<size_t> cutDist(1, n - 1);
    const size_t cut = cutDist(rng);

    std::vector<int> child1Genotype = parent1.genotype;
    std::vector<int> child2Genotype = parent2.genotype;

    for (size_t i = cut; i < n; ++i) {
        std::swap(child1Genotype[i], child2Genotype[i]);
    }

    return {Individual(std::move(child1Genotype)), Individual(std::move(child2Genotype))};
}

} // namespace cvrp
