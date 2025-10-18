#!/bin/bash
# Script to run individual algorithm tests on both datasets
# Author: Vector Similarity Search Project
# Purpose: Test all algorithms as required by project PDFs

echo "==============================================="
echo "Vector Similarity Search - Individual Tests"
echo "Implementation of PDF Requirements:"
echo "- Approximation Factor (Μέσο κλάσμα προσέγγισης)"
echo "- Recall@N (Ανάκληση στους Ν πλησιέστερους γείτονες)"
echo "- Queries Per Second (Πλήθος αναζητήσεων ανά δευτερόλεπτο)"
echo "==============================================="

# Ensure results directory exists
mkdir -p results

echo
echo "Testing on SIFT Dataset (128-dimensional SIFT features)..."

echo "1. LSH Algorithm on SIFT..."
./search -d dataset/sift_base.fvecs -q dataset/sift_query.fvecs -o results/sift_lsh.txt -type sift -lsh -k 4 -L 5 -w 4.0 -N 10 -R 2.0 -range true

echo "2. Hypercube Algorithm on SIFT..."
./search -d dataset/sift_base.fvecs -q dataset/sift_query.fvecs -o results/sift_hypercube.txt -type sift -hypercube -kproj 14 -w 4.0 -M 10 -probes 2 -N 10 -R 2.0 -range true

echo "3. IVFFlat Algorithm on SIFT..."
./search -d dataset/sift_base.fvecs -q dataset/sift_query.fvecs -o results/sift_ivfflat.txt -type sift -ivfflat -kclusters 50 -nprobe 5 -N 10 -R 2.0 -range true -seed 1

echo "4. IVFPQ Algorithm on SIFT..."
./search -d dataset/sift_base.fvecs -q dataset/sift_query.fvecs -o results/sift_ivfpq.txt -type sift -ivfpq -kclusters 50 -nprobe 5 -M 16 -nbits 8 -N 10 -R 2.0 -range true -seed 1

echo
echo "Testing on MNIST Dataset (784-dimensional handwritten digits)..."

echo "5. LSH Algorithm on MNIST..."
./search -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/mnist_lsh.txt -type mnist -lsh -k 4 -L 5 -w 10.0 -N 10 -R 2000.0 -range true

echo "6. Hypercube Algorithm on MNIST..."
./search -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/mnist_hypercube.txt -type mnist -hypercube -kproj 14 -w 10.0 -M 10 -probes 2 -N 10 -R 2000.0 -range true

echo "7. IVFFlat Algorithm on MNIST..."
./search -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/mnist_ivfflat.txt -type mnist -ivfflat -kclusters 50 -nprobe 5 -N 10 -R 2000.0 -range true -seed 1

echo "8. IVFPQ Algorithm on MNIST..."
./search -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/mnist_ivfpq.txt -type mnist -ivfpq -kclusters 50 -nprobe 5 -M 16 -nbits 8 -N 10 -R 2000.0 -range true -seed 1

echo
echo "==============================================="
echo "All individual tests completed!"
echo "Results are saved in the results/ directory:"
echo "- sift_*.txt: SIFT dataset results"
echo "- mnist_*.txt: MNIST dataset results"
echo
echo "For comprehensive experimental comparison study:"
echo "Run: make comprehensive-benchmark"
echo "==============================================="