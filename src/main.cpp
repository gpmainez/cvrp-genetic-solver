#include "cvrp/InstanceLoader.h"
#include "cvrp/Evaluator.h"
#include "cvrp/GeneticAlgorithm.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <filesystem>

void printBanner() {
    std::cout << "=========================================================\n"
              << "   CVRP High-Performance Genetic Algorithm Solver (C++20)\n"
              << "=========================================================\n";
}

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [options]\n\n"
              << "Options:\n"
              << "  --file <path>       Path to .lcvrp instance file (default: data/A-n33-k5.lcvrp)\n"
              << "  --pop <size>        Population size (default: 100)\n"
              << "  --gens <count>      Number of generations (default: 1000)\n"
              << "  --cross <prob>      Crossover probability (default: 0.80)\n"
              << "  --mut <prob>        Mutation probability per gene (default: 0.02)\n"
              << "  --threads <n>       Number of worker threads (default: hardware concurrency)\n"
              << "  --quiet             Suppress periodic generation logs\n"
              << "  --help, -h          Display this help message\n\n";
}

int main(int argc, char* argv[]) {
    printBanner();

    std::filesystem::path filePath = "data/A-n33-k5.lcvrp";
    cvrp::GAConfig config;
    config.verbose = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--file" && i + 1 < argc) {
            filePath = argv[++i];
        } else if (arg == "--pop" && i + 1 < argc) {
            config.populationSize = std::stoi(argv[++i]);
        } else if (arg == "--gens" && i + 1 < argc) {
            config.maxGenerations = std::stoi(argv[++i]);
        } else if (arg == "--cross" && i + 1 < argc) {
            config.crossoverProb = std::stod(argv[++i]);
        } else if (arg == "--mut" && i + 1 < argc) {
            config.mutationProb = std::stod(argv[++i]);
        } else if (arg == "--threads" && i + 1 < argc) {
            config.threadCount = std::stoi(argv[++i]);
        } else if (arg == "--quiet") {
            config.verbose = false;
        }
    }

    if (!std::filesystem::exists(filePath)) {
        std::cerr << "[Error] Benchmark file not found: " << filePath << "\n";
        printUsage(argv[0]);
        return 1;
    }

    std::cout << "[INFO] Loading instance: " << filePath.string() << " ...\n";
    auto instanceOpt = cvrp::InstanceLoader::loadFromFile(filePath);
    if (!instanceOpt.has_value()) {
        std::cerr << "[Error] Failed to parse instance file: " << filePath << "\n";
        return 1;
    }

    const auto& instance = instanceOpt.value();
    std::cout << "[INFO] Instance: " << instance.getName()
              << " | Dimension: " << instance.getDimension()
              << " | Clients: " << instance.getClientCount()
              << " | Vehicles: " << instance.getVehicleCount()
              << " | Capacity: " << instance.getCapacity() << "\n";

    std::cout << "[INFO] Config: PopSize=" << config.populationSize
              << ", Gens=" << config.maxGenerations
              << ", CrossProb=" << config.crossoverProb
              << ", MutProb=" << config.mutationProb
              << ", Threads=" << (config.threadCount == 0 ? "auto" : std::to_string(config.threadCount))
              << "\n\n";

    cvrp::Evaluator evaluator(instance);
    cvrp::GeneticAlgorithm ga(evaluator, config);

    std::cout << "Starting optimization...\n";
    cvrp::Individual best = ga.solve();

    std::cout << "\n====================== RESULTS ==========================\n"
              << "Best Total Cost (Distance): " << std::fixed << std::setprecision(2) << best.getFitness() << "\n"
              << "Execution Time:            " << ga.getExecutionTimeMs() << " ms\n"
              << "=========================================================\n\n";

    std::cout << "Vehicle Routes Breakdown:\n";
    auto routes = evaluator.getDetailedRoutes(best);
    for (const auto& r : routes) {
        if (r.trips.empty()) continue;
        std::cout << "Vehicle #" << r.vehicleId
                  << " (Distance: " << r.totalDistance
                  << ", Total Demand: " << r.totalDemand << "):\n";

        for (size_t tripIdx = 0; tripIdx < r.trips.size(); ++tripIdx) {
            const auto& trip = r.trips[tripIdx];
            std::cout << "  Trip " << (tripIdx + 1) << " [Load " << trip.totalDemand << "/" << instance.getCapacity() << "]: ";
            for (size_t n = 0; n < trip.nodes.size(); ++n) {
                std::cout << trip.nodes[n];
                if (n + 1 < trip.nodes.size()) std::cout << " -> ";
            }
            std::cout << " (Cost: " << trip.distance << ")\n";
        }
    }
    std::cout << "=========================================================\n";

    return 0;
}
