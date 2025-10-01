#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>

/** 
 Mathematical functions that are useful for our project
 and C++ does not offer (or offers under a different definition) 
 **/

namespace our_math {
    // modulo that works for every integer number
    inline int modulo(int a, int b);

    // executes the operation (x^y) mod p
    inline int modular_power(int x, int y, int p);
    
    // Convert a number from big endian to little endian
    inline int big_to_little_endian(int i);

    // Bitwise concatenate 2 integers
    inline uint32_t bitwise_concat(uint16_t current, uint32_t next, int k);
    
    // compute the best w value
    inline double compute_w_value(std::vector<std::vector<double>> feature_vectors, int n_images);

    // find all of the numbers that have hamming distance = 1 with a given number of bitwise size k
    inline std::list<int> hamming_distance_1(int number, int k);

    // find the hamming distance between 2 integers
    inline int hamming_distance(int a, int b);
    
    template <typename T>
    // add the vector b to a
    inline std::vector<T> add_vectors(std::vector<T> a, std::vector<T> b);

    template <typename T>
    // divide a vector by an integer
    inline std::vector<T> divide(std::vector<T> a, int k);

    template <typename T>
    // find the median member of a vector
    inline T median(std::vector<T> a);
}