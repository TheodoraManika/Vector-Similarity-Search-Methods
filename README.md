# Vector-Similarity-Search-Methods

Responsibilities Theodora:
- [ ] Random Projection
- [ ] IVFPQ
- [ ] SIFT

Responsibilities Myrto:
- [ ] LSH
- [ ] kmeans
- [ ] IVFFlat
- [ ] MNIST

To do Theodora:
- [ ]

To do Myrto:
- [ ]


Θα έχουμε ένα γενικό hashtable και δύο subclasses για το lsh και το hypercube 


# Εγκατάσταση (Ubuntu/Debian)
sudo apt update
sudo apt install -y build-essential cmake ninja-build gdb

# Από τον ριζικό φάκελο του project σου:
cmake --preset dev-debug
cmake --build --preset build-debug -j

# Τρέξε το εκτελέσιμο
./build/debug/vss_app

# Tests
ctest --preset test-debug


https://github.com/nikosgalanis/Vectors-Similarity-Search-and-Clustering/blob/main/results/cluster_lloyds_1000_queries