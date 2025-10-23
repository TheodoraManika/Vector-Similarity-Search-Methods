#include "dataprocessing/SIFT/SIFT.h"
#include "metrics/metrics.h"
#include <iostream>

int main() {
    auto dataset = SIFT::parseSIFTFile("dataset/sift_learn.fvecs");
    auto queries = SIFT::parseSIFTFile("dataset/sift_query.fvecs");
    
    // Check first query
    float dist = metrics::EuclideanDistance(queries[0], dataset[21], 128);
    std::cout << "Distance between query[0] and dataset[21]: " << dist << std::endl;
    
    // Check if they're actually the same
    bool same = true;
    for (int i = 0; i < 128; i++) {
        if (queries[0][i] != dataset[21][i]) {
            same = false;
            break;
        }
    }
    std::cout << "Are they identical? " << (same ? "YES" : "NO") << std::endl;
    
    return 0;
}
