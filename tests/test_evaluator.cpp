#include <gtest/gtest.h>
#include "cvrp/ProblemInstance.h"
#include "cvrp/Evaluator.h"

TEST(EvaluatorTest, SyntheticInstanceEvaluation) {
    // 1 depot (0,0) and 2 clients: client 1 at (0, 3) with demand 10, client 2 at (4, 0) with demand 10
    // Vehicle capacity 15 (cannot carry both on a single trip), vehicle count 1
    std::vector<cvrp::Point> coords = {
        {0.0, 0.0}, // Depot 0
        {0.0, 3.0}, // Client 1
        {4.0, 0.0}  // Client 2
    };
    std::vector<int> demands = {0, 10, 10};
    std::vector<int> perm = {1, 2};

    cvrp::ProblemInstance instance(
        "synthetic", 3, 15, 1, 0,
        coords, demands, perm
    );

    cvrp::Evaluator evaluator(instance);

    // Genotype: both assigned to vehicle 0
    std::vector<int> genotype = {0, 0};
    cvrp::Individual ind(genotype);

    // Expected trips:
    // Trip 1: Depot(0) -> Client 1 (dist: 3), demand 10. Next client demands 10, 10+10 > 15 -> return to Depot (dist: 3). Trip 1 cost = 6.
    // Trip 2: Depot(0) -> Client 2 (dist: 4), demand 10. Return to Depot (dist: 4). Trip 2 cost = 8.
    // Total cost = 14.
    double cost = evaluator.evaluate(ind);
    EXPECT_DOUBLE_EQ(cost, 14.0);

    auto routes = evaluator.getDetailedRoutes(ind);
    ASSERT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0].trips.size(), 2);
    EXPECT_DOUBLE_EQ(routes[0].totalDistance, 14.0);
}

TEST(EvaluatorTest, ParallelEvaluationMatchesSequential) {
    std::vector<cvrp::Point> coords = {
        {0.0, 0.0}, {10.0, 0.0}, {0.0, 10.0}, {10.0, 10.0}
    };
    std::vector<int> demands = {0, 5, 5, 5};
    std::vector<int> perm = {1, 2, 3};

    cvrp::ProblemInstance instance("test", 4, 10, 2, 0, coords, demands, perm);
    cvrp::Evaluator evaluator(instance);

    std::vector<cvrp::Individual> popSequential;
    std::vector<cvrp::Individual> popParallel;

    for (int i = 0; i < 20; ++i) {
        std::vector<int> g = {i % 2, (i + 1) % 2, i % 2};
        popSequential.emplace_back(g);
        popParallel.emplace_back(g);
    }

    evaluator.evaluatePopulation(popSequential, 1); // 1 thread
    evaluator.evaluatePopulation(popParallel, 4);   // 4 threads

    for (size_t i = 0; i < popSequential.size(); ++i) {
        EXPECT_DOUBLE_EQ(popSequential[i].getFitness(), popParallel[i].getFitness());
    }
}
