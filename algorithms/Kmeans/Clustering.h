#pragma once

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <utility>
#include <climits>

#include "LSH/LSH.h"
#include "hypercube/Hypercube.h"

#define UNASSIGNED -1

template <typename T>
class Clustering {
    private:
        int k; // number of clusters needed
        int n_points; //number of points in the dataset
        uint32_t space_dim; // space dimension

        /** assignment method chosen by the user. Options are:
              - lloyds
              - reverse_LSH
              - reverse_Hypercube 
         **/
        std::string assignment_method;
        
        // vectors of the centroids
        std::vector<std::vector<T>> centroids;

        // vector of the vectors given to us, stored here for convenience
        std::vector<std::vector<T>> feature_vectors;

        // vector that holds the index of the centroid that the current index's vector is assigned to
        std::vector<int> assigned_centroid;

        // in case of lsh, we need these extra variables
        uint32_t lsh_l;
        uint32_t lsh_k;
        LSH<T>* lsh_instant;

        // in case of hypercube, we need these extra variables
        uint32_t hc_M;
        uint32_t hc_k;
        uint32_t hc_probes;
        Hypercube<T>* hc_instant;

        //compute the nearest center for a given vector
        int nearest_centroid(std::vector<T> vec);

        // compute the second nearest centroid of a vector. Needed for the silhouette algorithm
        int second_nearest_centroid(std::vector<T> vec);

        // compute how many unassigned vectors we've got, for the reverse assignment method
        int count_unassigned(std::vector<int> assigned);

        // compute the minimum of the distances of the centroids. Needed for the initialization of the radius in reverse assignment
        T min_dist_between_centroids();

    public:
        // constructor when lloyds assignment is requested
        Clustering(std::string assignment_method, std::vector<std::vector<T>> feature_vectors, uint32_t k);

        // constructor when reverse assignment using lsh is requested
        Clustering(std::string assignment_method, std::vector<std::vector<T>> feature_vectors,
                   uint32_t k, uint32_t lsh_l, uint32_t lsh_k);

        // constructor when reverse assignment using hypercube is requested
        Clustering(std::string assignment_method, std::vector<std::vector<T>> feature_vectors, uint32_t k,
                   uint32_t hc_M, uint32_t hc_k, uint32_t hc_probes);

        // destructor for the clustering class
        ~Clustering();

        // get the vector of the centroids
        std::vector<std::vector<T>> get_centroids();

        std::vector<std::vector<int>> get_centroids_map();
        
        // initialize our clusters using the init++ method
        void initialization(void);

        // update the centroids with the kmedian method
        void update(void);

        // assign vectors to the nearest cluster using the lloyds algorithm
        int assignment_lloyds(void);
        
        int reverse_assignment(void);

        // compute the silhouette of each cluster, and the total one
        std::pair<std::vector<double>, double> compute_silhouette(void);

        // run the clustering algorithm
        void run_clustering(void);
};