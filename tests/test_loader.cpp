#include <gtest/gtest.h>
#include "cvrp/InstanceLoader.h"
#include <filesystem>

TEST(InstanceLoaderTest, ParseVehicleCountFromFilename) {
    EXPECT_EQ(cvrp::InstanceLoader::parseVehicleCount("A-n33-k5.lcvrp"), 5);
    EXPECT_EQ(cvrp::InstanceLoader::parseVehicleCount("B-n50-k8.vrp"), 8);
    EXPECT_EQ(cvrp::InstanceLoader::parseVehicleCount("no_k_here.txt"), 0);
}

TEST(InstanceLoaderTest, LoadValidInstance) {
    const std::filesystem::path testFile = "data/A-n33-k5.lcvrp";
    if (!std::filesystem::exists(testFile)) {
        GTEST_SKIP() << "Benchmark file data/A-n33-k5.lcvrp not found in working directory.";
    }

    auto instanceOpt = cvrp::InstanceLoader::loadFromFile(testFile);
    ASSERT_TRUE(instanceOpt.has_value());

    const auto& instance = instanceOpt.value();
    EXPECT_EQ(instance.getDimension(), 33);
    EXPECT_EQ(instance.getClientCount(), 32);
    EXPECT_EQ(instance.getCapacity(), 100);
    EXPECT_EQ(instance.getVehicleCount(), 5);
    EXPECT_EQ(instance.getDepotIndex(), 0);

    // Depot demand should be 0
    EXPECT_EQ(instance.getDemand(0), 0);

    // Distance matrix properties
    EXPECT_EQ(instance.getDistance(0, 0), 0);
    EXPECT_EQ(instance.getDistance(1, 2), instance.getDistance(2, 1));
    EXPECT_GT(instance.getDistance(0, 1), 0);
}

TEST(InstanceLoaderTest, HandleNonexistentFile) {
    auto instanceOpt = cvrp::InstanceLoader::loadFromFile("invalid_path_123.lcvrp");
    EXPECT_FALSE(instanceOpt.has_value());
}
