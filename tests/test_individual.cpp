#include <gtest/gtest.h>
#include "cvrp/Individual.h"
#include <random>

TEST(IndividualTest, GenotypeAndFitnessState) {
    std::vector<int> genotype = {0, 1, 2, 0, 1};
    cvrp::Individual ind(genotype);

    EXPECT_EQ(ind.getGenotype(), genotype);
    EXPECT_FALSE(ind.isEvaluated());

    ind.setFitness(420.5);
    EXPECT_TRUE(ind.isEvaluated());
    EXPECT_DOUBLE_EQ(ind.getFitness(), 420.5);

    ind.invalidateFitness();
    EXPECT_FALSE(ind.isEvaluated());
}

TEST(IndividualTest, CrossoverProducesValidChildren) {
    std::vector<int> g1 = {0, 0, 0, 0, 0};
    std::vector<int> g2 = {1, 1, 1, 1, 1};

    cvrp::Individual p1(g1);
    cvrp::Individual p2(g2);

    std::mt19937_64 rng(42);
    auto [c1, c2] = cvrp::Individual::crossover(p1, p2, 1.0, rng);

    EXPECT_EQ(c1.getGenotype().size(), g1.size());
    EXPECT_EQ(c2.getGenotype().size(), g2.size());
    EXPECT_FALSE(c1.isEvaluated());
    EXPECT_FALSE(c2.isEvaluated());
}

TEST(IndividualTest, MutationAltersGenotypeAndInvalidatesFitness) {
    std::vector<int> genotype(20, 0);
    cvrp::Individual ind(genotype);
    ind.setFitness(100.0);

    std::mt19937_64 rng(1337);
    // Mutate with 100% gene mutation probability across 5 groups
    ind.mutate(1.0, 5, rng);

    EXPECT_FALSE(ind.isEvaluated());
    bool anyChanged = false;
    for (int gene : ind.getGenotype()) {
        if (gene != 0) anyChanged = true;
    }
    EXPECT_TRUE(anyChanged);
}
