#include <iostream>
#include <memory>
#include <chrono>
#include <iomanip>
#include "utils/interface/Interface.h"
#include "algorithms/LSH/LSH.h"
#include "algorithms/hypercube/Hypercube.h"
#include "algorithms/IVFFlat/IVFFlat.h"
#include "algorithms/IVFPQ/IVFPQ.h"
#include "dataprocessing/SIFT/SIFT.h"
#include "dataprocessing/MNIST/MNIST.h"
#include "metrics/metrics.h"
#include "utils/helpers/Utils.h"

int main(int argc, char* argv[]) {
    try {
        // Create interface instance and run search
        Interface interface;
        return interface.runSearch(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}