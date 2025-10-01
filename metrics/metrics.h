#pragma once

#include <iostream>
#include <cmath>
#include <vector>

// Various metrics for vectors
namespace metrics {
    template <typename T> 
    // Compute the Manhattan Distance between 2 vectors in the dim-th dimension
    T ManhattanDistance(std::vector<T> x, std::vector<T> y, int dim);

    template <typename T> 
    // Compute the Euclidean Distance between 2 vectors in the dim-th dimension
    T EuclideanDistance(std::vector<T> x, std::vector<T> y, int dim);
};