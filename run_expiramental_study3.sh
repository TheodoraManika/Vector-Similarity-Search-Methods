#!/bin/bash

# Experimental Study 3: OPTIMIZED LSH parameters based on study2 results
# This script uses only the best-performing LSH parameters from study2
# while keeping all other algorithm tests unchanged

set -euo pipefail

RESULTS_DIR="results/report_tests"
mkdir -p "$RESULTS_DIR" "${RESULTS_DIR}/lsh" "${RESULTS_DIR}/hypercube" "${RESULTS_DIR}/ivfflat" "${RESULTS_DIR}/ivfpq"

# Datasets (override with env vars if needed)
MNIST_TRAIN=${MNIST_TRAIN:-dataset/train-images.idx3-ubyte}
MNIST_QUERY=${MNIST_QUERY:-dataset/t10k-images.idx3-ubyte}
SIFT_TRAIN=${SIFT_TRAIN:-dataset/sift_learn.fvecs}
SIFT_QUERY=${SIFT_QUERY:-dataset/sift_query.fvecs}

# Number of queries to process (0 = all)
QMAX=${QMAX:-100}

# Number of parallel jobs (default: number of CPU cores)
MAX_JOBS=${MAX_JOBS:-$(nproc)}

# Array to track background jobs
declare -a PIDS=()

run_parallel() {
  echo "[$(date +%H:%M:%S)] $*"
  eval "$*"
}

# Run command in background with job control
run_parallel() {
  # Wait if we've hit the max number of parallel jobs
  while [ ${#PIDS[@]} -ge $MAX_JOBS ]; do
    for i in "${!PIDS[@]}"; do
      if ! kill -0 "${PIDS[$i]}" 2>/dev/null; then
        # Job finished, remove from array
        wait "${PIDS[$i]}" 2>/dev/null || true
        unset 'PIDS[$i]'
      fi
    done
    PIDS=("${PIDS[@]}") # Re-index array
    sleep 0.1
  done
  
  # Start the command in background
  echo "[$(date +%H:%M:%S)] [PARALLEL] $*"
  eval "$* &"
  PIDS+=($!)
}

# Wait for all background jobs to complete
wait_all() {
  echo "[$(date +%H:%M:%S)] Waiting for ${#PIDS[@]} remaining jobs to complete..."
  for pid in "${PIDS[@]}"; do
    wait "$pid" 2>/dev/null || true
  done
  PIDS=()
  echo "[$(date +%H:%M:%S)] All jobs completed!"
}

echo "Running optimized tests with QMAX=${QMAX}, MAX_JOBS=${MAX_JOBS} parallel jobs"
echo "Set QMAX or MAX_JOBS env vars to change (e.g., MAX_JOBS=4 ./run_expiramental_study3.sh)"

########################################
# LSH — MNIST (OPTIMIZED)
# All parameters from study2 achieved 100% recall
# Selecting best speed/recall combinations
########################################
if [[ -f "$MNIST_TRAIN" && -f "$MNIST_QUERY" ]]; then
  echo "=== LSH MNIST (OPTIMIZED - 100% Recall) ==="
  
  # BEST OVERALL: High speed + 100% recall
  echo "  [BEST] Fastest with perfect recall..."
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L100_k2_w5000_N5_BEST.txt" -type mnist -lsh -k 2 -L 100 -w 5000 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L100_k3_w4500_N5.txt" -type mnist -lsh -k 3 -L 100 -w 4500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  
  # Speed/Quality tradeoffs
  echo "  [TRADEOFF] Different speed/quality tradeoffs..."
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L80_k2_w4500_N5.txt" -type mnist -lsh -k 2 -L 80 -w 4500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L100_k2_w4500_N5.txt" -type mnist -lsh -k 2 -L 100 -w 4500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L100_k3_w4000_N5.txt" -type mnist -lsh -k 3 -L 100 -w 4000 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L120_k3_w4000_N5.txt" -type mnist -lsh -k 3 -L 120 -w 4000 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  
  # Higher quality (L=150) for maximum robustness
  echo "  [HIGH QUALITY] Maximum L for most robust results..."
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L150_k2_w4500_N5.txt" -type mnist -lsh -k 2 -L 150 -w 4500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L150_k3_w4000_N5.txt" -type mnist -lsh -k 3 -L 150 -w 4000 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  
  # N=10 tests for best configurations
  echo "  [N=10] Testing with more neighbors returned..."
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L100_k2_w4500_N10.txt" -type mnist -lsh -k 2 -L 100 -w 4500 -N 10 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L120_k3_w4000_N10.txt" -type mnist -lsh -k 3 -L 120 -w 4000 -N 10 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L150_k2_w4500_N10.txt" -type mnist -lsh -k 2 -L 150 -w 4500 -N 10 -R 500 -range true -qmax $QMAX -auto_w false
else
  echo "[WARN] MNIST files not found: $MNIST_TRAIN / $MNIST_QUERY — skipping LSH MNIST"
fi

########################################
# LSH — SIFT (OPTIMIZED)
# Best 90% recall configurations from study2
########################################
if [[ -f "$SIFT_TRAIN" && -f "$SIFT_QUERY" ]]; then
  echo "=== LSH SIFT (OPTIMIZED - 90% Recall Target) ==="
  
  # BEST OVERALL: 90% recall with highest speed
  echo "  [BEST] 90% recall, fastest speed..."
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L100_k2_w650_N10_BEST.txt" -type sift -lsh -k 2 -L 100 -w 650 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  
  # Alternative 90% recall configurations
  echo "  [90% RECALL] Alternative configurations..."
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L120_k3_w600_N10.txt" -type sift -lsh -k 3 -L 120 -w 600 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L150_k2_w650_N10.txt" -type sift -lsh -k 2 -L 150 -w 650 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  
  # Good 80% recall configurations (faster, still good quality)
  echo "  [80% RECALL] Faster configurations with good recall..."
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L80_k2_w650_N5.txt" -type sift -lsh -k 2 -L 80 -w 650 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L80_k3_w600_N5.txt" -type sift -lsh -k 3 -L 80 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L100_k2_w650_N5.txt" -type sift -lsh -k 2 -L 100 -w 650 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L100_k3_w600_N5.txt" -type sift -lsh -k 3 -L 100 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  
  # Parameter variations for comprehensive comparison
  echo "  [VARIATIONS] Additional parameter combinations..."
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L100_k2_w700_N5.txt" -type sift -lsh -k 2 -L 100 -w 700 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L100_k3_w650_N5.txt" -type sift -lsh -k 3 -L 100 -w 650 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L120_k2_w650_N5.txt" -type sift -lsh -k 2 -L 120 -w 650 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L120_k3_w600_N5.txt" -type sift -lsh -k 3 -L 120 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L150_k2_w650_N5.txt" -type sift -lsh -k 2 -L 150 -w 650 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L150_k3_w600_N5.txt" -type sift -lsh -k 3 -L 150 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
else
  echo "[WARN] SIFT files not found: $SIFT_TRAIN / $SIFT_QUERY — skipping LSH SIFT"
fi

########################################
# Hypercube — MNIST (from study2 - UNCHANGED)
########################################
if [[ -f "$MNIST_TRAIN" && -f "$MNIST_QUERY" ]]; then
  echo "=== Hypercube MNIST ==="
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M1000_p100_w3500_N5.txt" -type mnist -hypercube -kproj 14 -M 1000 -probes 100 -w 3500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M2000_p1000_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 2000 -probes 1000 -w 2500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M5000_p2000_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 5000 -probes 2000 -w 2500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M10000_p1000_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 10000 -probes 1000 -w 2500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M15000_p3000_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 15000 -probes 3000 -w 2500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M20000_p3000_w3000_N5.txt" -type mnist -hypercube -kproj 14 -M 20000 -probes 3000 -w 3000 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k15_M20000_p10000_w2500_N5.txt" -type mnist -hypercube -kproj 15 -M 20000 -probes 10000 -w 2500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M20000_p5500_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 20000 -probes 5500 -w 2500 -N 5 -R 500 -range true -qmax $QMAX -auto_w false
  # N=10 variants
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M20000_p3000_w3000_N10.txt" -type mnist -hypercube -kproj 14 -M 20000 -probes 3000 -w 3000 -N 10 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k15_M20000_p10000_w2500_N10.txt" -type mnist -hypercube -kproj 15 -M 20000 -probes 10000 -w 2500 -N 10 -R 500 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M20000_p5500_w2500_N10.txt" -type mnist -hypercube -kproj 14 -M 20000 -probes 5500 -w 2500 -N 10 -R 500 -range true -qmax $QMAX -auto_w false
else
  echo "[WARN] MNIST files not found — skipping Hypercube MNIST"
fi

########################################
# Hypercube — SIFT (from study2 - UNCHANGED)
########################################
if [[ -f "$SIFT_TRAIN" && -f "$SIFT_QUERY" ]]; then
  echo "=== Hypercube SIFT ==="
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k14_M200000_p8000_w600_N5.txt" -type sift -hypercube -kproj 14 -M 200000 -probes 8000 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M200000_p10000_w600_N5.txt" -type sift -hypercube -kproj 15 -M 200000 -probes 10000 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M200000_p10000_w500_N5.txt" -type sift -hypercube -kproj 15 -M 200000 -probes 10000 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M200000_p10000_w400_N5.txt" -type sift -hypercube -kproj 15 -M 200000 -probes 10000 -w 400 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M250000_p10000_w500_N5.txt" -type sift -hypercube -kproj 15 -M 250000 -probes 10000 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k14_M250000_p10000_w500_N5.txt" -type sift -hypercube -kproj 14 -M 250000 -probes 10000 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  # N=10 variants
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M200000_p10000_w500_N10.txt" -type sift -hypercube -kproj 15 -M 200000 -probes 10000 -w 500 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M250000_p10000_w500_N10.txt" -type sift -hypercube -kproj 15 -M 250000 -probes 10000 -w 500 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k14_M250000_p10000_w500_N10.txt" -type sift -hypercube -kproj 14 -M 250000 -probes 10000 -w 500 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
else
  echo "[WARN] SIFT files not found — skipping Hypercube SIFT"
fi

########################################
# IVFFlat — MNIST (from study2 - UNCHANGED)
########################################
if [[ -f "$MNIST_TRAIN" && -f "$MNIST_QUERY" ]]; then
  echo "=== IVFFlat MNIST ==="
  # From silhouette selection: k=37, nprobe sweep
  for NPROBE in 1 2 3 4 5 6 7 8 9 10 15; do
   run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/ivfflat/mnist_k37_p${NPROBE}_N5.txt" -type mnist -ivfflat -kclusters 37 -nprobe ${NPROBE} -N 5 -R 500 -range true -qmax $QMAX
  done
  # N=5,10 for nprobe=5
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/ivfflat/mnist_k37_p5_N10.txt" -type mnist -ivfflat -kclusters 37 -nprobe 5 -N 10 -R 500 -range true -qmax $QMAX
else
  echo "[WARN] MNIST files not found — skipping IVFFlat MNIST"
fi

########################################
# IVFFlat — SIFT (from study2 - UNCHANGED)
########################################
if [[ -f "$SIFT_TRAIN" && -f "$SIFT_QUERY" ]]; then
  echo "=== IVFFlat SIFT ==="
  # From silhouette selection: k=87, nprobe sweep
  for NPROBE in 1 2 3 4 5 6 7 8 9 10 15 20; do
    run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k87_p${NPROBE}_N5.txt" -type sift -ivfflat -kclusters 87 -nprobe ${NPROBE} -N 5 -R 300 -range true -qmax $QMAX
  done
  # N=10 for nprobe=10
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k87_p10_N10.txt" -type sift -ivfflat -kclusters 87 -nprobe 10 -N 10 -R 300 -range true -qmax $QMAX
else
  echo "[WARN] SIFT files not found — skipping IVFFlat SIFT"
fi

########################################
# IVFPQ — MNIST (from study2 - UNCHANGED)
########################################
if [[ -f "$MNIST_TRAIN" && -f "$MNIST_QUERY" ]]; then
  echo "=== IVFPQ MNIST ==="
  # k=37 (from IVFFlat silhouette), m sweep, nprobe=5
  for M in 4 8 16; do
    run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/ivfpq/mnist_k37_m${M}_b8_p5_N5.txt" -type mnist -ivfpq -kclusters 37 -nprobe 5 -m ${M} -nbits 8 -N 5 -R 500 -range true -qmax $QMAX
  done
  # m=8, nprobe sweep
  for NPROBE in 3 5 7 10; do
    run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/ivfpq/mnist_k37_m8_b8_p${NPROBE}_N5.txt" -type mnist -ivfpq -kclusters 37 -nprobe ${NPROBE} -m 8 -nbits 8 -N 5 -R 500 -range true -qmax $QMAX
  done
  # N=10 for best config
  run_parallel ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/ivfpq/mnist_k37_m8_b8_p5_N10.txt" -type mnist -ivfpq -kclusters 37 -nprobe 5 -m 8 -nbits 8 -N 10 -R 500 -range true -qmax $QMAX
else
  echo "[WARN] MNIST files not found — skipping IVFPQ MNIST"
fi

########################################
# IVFPQ — SIFT (from study2 - UNCHANGED)
########################################
if [[ -f "$SIFT_TRAIN" && -f "$SIFT_QUERY" ]]; then
  echo "=== IVFPQ SIFT ==="
  # k=87 (from IVFFlat silhouette), m sweep, nprobe=10
  for M in 4 8 16; do
    run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfpq/sift_k87_m${M}_b8_p10_N5.txt" -type sift -ivfpq -kclusters 87 -nprobe 10 -m ${M} -nbits 8 -N 5 -R 300 -range true -qmax $QMAX
  done
  # m=8, nprobe sweep
  for NPROBE in 5 10 15 20; do
    run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfpq/sift_k87_m8_b8_p${NPROBE}_N5.txt" -type sift -ivfpq -kclusters 87 -nprobe ${NPROBE} -m 8 -nbits 8 -N 5 -R 300 -range true -qmax $QMAX
  done
  # N=10 for best config
  run_parallel ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfpq/sift_k87_m8_b8_p10_N10.txt" -type sift -ivfpq -kclusters 87 -nprobe 10 -m 8 -nbits 8 -N 10 -R 300 -range true -qmax $QMAX
else
  echo "[WARN] SIFT files not found — skipping IVFPQ SIFT"
fi

########################################
# Wait for all jobs to complete
########################################
wait_all

echo "============================================"
echo "All tests completed!"
echo "Results are in: $RESULTS_DIR"
echo "============================================"
echo ""
echo "LSH OPTIMIZATIONS APPLIED:"
echo "  MNIST: All configs achieve 100% recall"
echo "    - BEST: L=100, k=2, w=5000 (QPS: 1.78)"
echo "    - Also good: L=100, k=3, w=4500 (QPS: 1.70)"
echo ""
echo "  SIFT: Optimized for 90% recall"
echo "    - BEST: L=100, k=2, w=650, N=10 (QPS: 2.83, Recall: 90%)"
echo "    - Alternative: L=120, k=3, w=600, N=10 (QPS: 2.35, Recall: 90%)"
echo "    - High quality: L=150, k=2, w=650, N=10 (QPS: 1.91, Recall: 90%)"
echo "============================================"
