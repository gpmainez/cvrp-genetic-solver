#include "cvrp/GeneticAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>

namespace cvrp {

GeneticAlgorithm::GeneticAlgorithm(const Evaluator& evaluator, GAConfig config)
    : evaluator(evaluator), config(config) {}

void GeneticAlgorithm::initializePopulation() {
    population.clear();
    population.reserve(config.populationSize);

    const int clientCount = evaluator.getInstance().getClientCount();
    const int vehicleCount = evaluator.getInstance().getVehicleCount();

    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int> groupDist(0, vehicleCount - 1);

    for (int i = 0; i < config.populationSize; ++i) {
        std::vector<int> genotype(clientCount);
        for (int& gene : genotype) {
            gene = groupDist(rng);
        }
        population.emplace_back(std::move(genotype));
    }
}

const Individual& GeneticAlgorithm::tournamentSelection(std::mt19937_64& rng) const {
    std::uniform_int_distribution<size_t> dist(0, population.size() - 1);

    size_t bestIdx = dist(rng);
    double bestFit = population[bestIdx].getFitness();

    for (int i = 1; i < config.tournamentSize; ++i) {
        const size_t candidateIdx = dist(rng);
        const double candidateFit = population[candidateIdx].getFitness();
        if (candidateFit < bestFit) {
            bestFit = candidateFit;
            bestIdx = candidateIdx;
        }
    }

    return population[bestIdx];
}

Individual GeneticAlgorithm::solve() {
    const auto startTime = std::chrono::high_resolution_clock::now();
    history.clear();

    initializePopulation();
    evaluator.evaluatePopulation(population, config.threadCount);
    std::sort(population.begin(), population.end());

    bestIndividual = population.front();

    const int vehicleCount = evaluator.getInstance().getVehicleCount();
    std::mt19937_64 rng(std::random_device{}());

    for (int gen = 0; gen < config.maxGenerations; ++gen) {
        std::vector<Individual> nextGen;
        nextGen.reserve(config.populationSize);

        // Elitism: Preserve best individuals without mutation
        const int eliteToKeep = std::min(config.elitismCount, static_cast<int>(population.size()));
        for (int e = 0; e < eliteToKeep; ++e) {
            nextGen.push_back(population[e]);
        }

        // Produce children
        while (nextGen.size() < static_cast<size_t>(config.populationSize)) {
            const auto& parent1 = tournamentSelection(rng);
            const auto& parent2 = tournamentSelection(rng);

            auto [child1, child2] = Individual::crossover(parent1, parent2, config.crossoverProb, rng);

            child1.mutate(config.mutationProb, vehicleCount, rng);
            nextGen.push_back(std::move(child1));

            if (nextGen.size() < static_cast<size_t>(config.populationSize)) {
                child2.mutate(config.mutationProb, vehicleCount, rng);
                nextGen.push_back(std::move(child2));
            }
        }

        population = std::move(nextGen);
        evaluator.evaluatePopulation(population, config.threadCount);
        std::sort(population.begin(), population.end());

        if (population.front().getFitness() < bestIndividual.getFitness()) {
            bestIndividual = population.front();
        }

        const double sumFitness = std::accumulate(
            population.begin(), population.end(), 0.0,
            [](double acc, const Individual& ind) { return acc + ind.getFitness(); }
        );
        const double avgFitness = sumFitness / population.size();

        history.push_back(GenerationStats{gen + 1, bestIndividual.getFitness(), avgFitness});

        if (config.verbose && ((gen + 1) % config.logInterval == 0 || (gen + 1) == config.maxGenerations)) {
            std::cout << "[Gen " << std::setw(5) << (gen + 1) << "/" << config.maxGenerations << "] "
                      << "Best: " << std::fixed << std::setprecision(2) << bestIndividual.getFitness()
                      << " | Avg: " << avgFitness << std::endl;
        }
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return bestIndividual;
}

} // namespace cvrp
