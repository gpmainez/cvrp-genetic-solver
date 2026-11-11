#pragma once

#include <vector>
#include <random>
#include <utility>

namespace cvrp {

class Individual {
public:
    Individual() = default;
    explicit Individual(std::vector<int> genotype);

    [[nodiscard]] const std::vector<int>& getGenotype() const noexcept { return genotype; }
    [[nodiscard]] std::vector<int>& getGenotype() noexcept { return genotype; }

    [[nodiscard]] double getFitness() const noexcept { return fitness; }
    void setFitness(double value) const noexcept {
        fitness = value;
        evaluated = true;
    }

    [[nodiscard]] bool isEvaluated() const noexcept { return evaluated; }
    void invalidateFitness() noexcept { evaluated = false; }

    void mutate(double geneMutProb, int vehicleCount, std::mt19937_64& rng);

    static std::pair<Individual, Individual> crossover(
        const Individual& parent1,
        const Individual& parent2,
        double crossProb,
        std::mt19937_64& rng
    );

    bool operator<(const Individual& other) const noexcept {
        return fitness < other.fitness;
    }

private:
    std::vector<int> genotype;
    mutable double fitness{1e9};
    mutable bool evaluated{false};
};

} // namespace cvrp
