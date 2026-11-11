#include <gtest/gtest.h>
#include "cvrp/ProblemInstance.h"
#include "cvrp/Evaluator.h"
#include "cvrp/GeneticAlgorithm.h"

TEST(GeneticAlgorithmTest, OptimizationMonotonicImprovement) {
    std::vector<cvrp::Point> coords = {
        {0.0, 0.0}, {10.0, 5.0}, {2.0, 8.0}, {7.0, 1.0}, {9.0, 9.0}
    };
    std::vector<int> demands = {0, 3, 4, 2, 5};
    std::vector<int> perm = {1, 2, 3, 4};

    cvrp::ProblemInstance instance("ga_test", 5, 10, 2, 0, coords, demands, perm);
    cvrp::Evaluator evaluator(instance);

    cvrp::GAConfig config;
    config.populationSize = 30;
    config.maxGenerations = 50;
    config.elitismCount = 2;
    config.verbose = false;

    cvrp::GeneticAlgorithm ga(evaluator, config);
    cvrp::Individual best = ga.solve();

    EXPECT_GT(best.getFitness(), 0.0);
    EXPECT_LT(best.getFitness(), 1e8); // Feasible solution found
    EXPECT_GT(ga.getExecutionTimeMs(), 0.0);

    const auto& history = ga.getHistory();
    ASSERT_EQ(history.size(), config.maxGenerations);

    // With elitism, best fitness should never increase
    for (size_t i = 1; i < history.size(); ++i) {
        EXPECT_LE(history[i].bestFitness, history[i - 1].bestFitness);
    }
}
