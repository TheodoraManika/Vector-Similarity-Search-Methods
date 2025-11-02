#!/bin/bash

# Πειραματική Συγκριτική Μελέτη
# Experimental Comparative Study Script
# Measures: AF (Approximation Factor), Recall@N, QPS (Queries Per Second)

echo "=========================================="
echo "ΠΕΙΡΑΜΑΤΙΚΗ ΣΥΓΚΡΙΤΙΚΗ ΜΕΛΕΤΗ"
echo "Experimental Comparative Study"
echo "=========================================="
echo ""

# Create results directory
mkdir -p results/experimental_study

# MNIST Dataset Parameters
MNIST_TRAIN="dataset/train-images.idx3-ubyte"
MNIST_QUERY="dataset/t10k-images.idx3-ubyte"
QMAX=100  # Number of queries to process

echo "Testing with QMAX=$QMAX queries"
echo ""

# ==================== LSH TESTS ====================
echo "=========================================="
echo "Testing LSH Algorithm"
echo "=========================================="

echo "LSH Test 1: k=4, L=5"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/lsh_k4_L5.txt \
  -type mnist -lsh -k 4 -L 5 -N 10 -qmax $QMAX

echo "LSH Test 2: k=6, L=10"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/lsh_k6_L10.txt \
  -type mnist -lsh -k 6 -L 10 -N 10 -qmax $QMAX

echo "LSH Test 3: k=8, L=15"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/lsh_k8_L15.txt \
  -type mnist -lsh -k 8 -L 15 -N 10 -qmax $QMAX

# ==================== HYPERCUBE TESTS ====================
echo ""
echo "=========================================="
echo "Testing Hypercube Algorithm"
echo "=========================================="

echo "Hypercube Test 1: kproj=14, M=5000, probes=1000"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/hypercube_M5000_p1000.txt \
  -type mnist -hypercube -kproj 14 -M 5000 -probes 1000 -N 10 -qmax $QMAX

echo "Hypercube Test 2: kproj=14, M=10000, probes=2000"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/hypercube_M10000_p2000.txt \
  -type mnist -hypercube -kproj 14 -M 10000 -probes 2000 -N 10 -qmax $QMAX

echo "Hypercube Test 3: kproj=14, M=20000, probes=5000"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/hypercube_M20000_p5000.txt \
  -type mnist -hypercube -kproj 14 -M 20000 -probes 5000 -N 10 -qmax $QMAX

# ==================== IVFFlat TESTS ====================
echo ""
echo "=========================================="
echo "Testing IVFFlat Algorithm"
echo "=========================================="

echo "IVFFlat Test 1: kclusters=50, nprobe=5"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/ivfflat_k50_p5.txt \
  -type mnist -ivfflat -kclusters 50 -nprobe 5 -N 10 -qmax $QMAX

echo "IVFFlat Test 2: kclusters=100, nprobe=10"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/ivfflat_k100_p10.txt \
  -type mnist -ivfflat -kclusters 100 -nprobe 10 -N 10 -qmax $QMAX

echo "IVFFlat Test 3: kclusters=150, nprobe=20"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/ivfflat_k150_p20.txt \
  -type mnist -ivfflat -kclusters 150 -nprobe 20 -N 10 -qmax $QMAX

# ==================== IVFPQ TESTS ====================
echo ""
echo "=========================================="
echo "Testing IVFPQ Algorithm"
echo "=========================================="

echo "IVFPQ Test 1: kclusters=50, nprobe=5, M=8"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/ivfpq_k50_p5_M8.txt \
  -type mnist -ivfpq -kclusters 50 -nprobe 5 -M 8 -nbits 8 -N 10 -qmax $QMAX

echo "IVFPQ Test 2: kclusters=100, nprobe=10, M=16"
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o results/experimental_study/ivfpq_k100_p10_M16.txt \
  -type mnist -ivfpq -kclusters 100 -nprobe 10 -M 16 -nbits 8 -N 10 -qmax $QMAX

echo ""
echo "=========================================="
echo "Experimental Study Complete!"
echo "=========================================="
echo ""
echo "Results saved in: results/experimental_study/"
echo ""
echo "Next steps:"
echo "1. Extract metrics from result files"
echo "2. Generate comparative analysis report"
echo "3. Create correlation plots"
