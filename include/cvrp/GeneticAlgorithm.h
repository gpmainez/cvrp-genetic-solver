#pragma once

#include "cvrp/Evaluator.h"
#include "cvrp/Individual.h"
#include <vector>
#include <random>
#include <chrono>
#include <functional>

namespace cvrp {

struct GAConfig {
    int populationSize{100};
    int maxGenerations{1000};
    double crossoverProb{0.8};
    double mutationProb{0.02};
    int tournamentSize{3};
    int elitismCount{2};
    int threadCount{0}; // 0 = hardware concurrency
    int logInterval{100};
    bool verbose{false};
};

struct GenerationStats {
    int generation{0};
    double bestFitness{0.0};
    double avgFitness{0.0};
};

class GeneticAlgorithm {
public:
    GeneticAlgorithm(const Evaluator& evaluator, GAConfig config = GAConfig{});

    Individual solve();

    [[nodiscard]] const Individual& getBestIndividual() const noexcept { return bestIndividual; }
    [[nodiscard]] const std::vector<GenerationStats>& getHistory() const noexcept { return history; }
    [[nodiscard]] double getExecutionTimeMs() const noexcept { return executionTimeMs; }
    [[nodiscard]] const GAConfig& getConfig() const noexcept { return config; }

private:
    void initializePopulation();
    [[nodiscard]] const Individual& tournamentSelection(std::mt19937_64& rng) const;

    const Evaluator& evaluator;
    GAConfig config;

    std::vector<Individual> population;
    Individual bestIndividual;
    std::vector<GenerationStats> history;
    double executionTimeMs{0.0};
};

} // namespace cvrp
