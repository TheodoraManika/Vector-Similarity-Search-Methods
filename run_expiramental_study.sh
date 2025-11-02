#!/bin/bash

# Run all tests listed in report.md and message.txt
# Outputs are stored under results/report_tests/<algorithm>/

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

run() {
  echo "[$(date +%H:%M:%S)] $*"
  eval "$*"
}

echo "Running tests with QMAX=${QMAX} (set QMAX env to change)"

########################################
# LSH — MNIST (from report)
########################################
if [[ -f "$MNIST_TRAIN" && -f "$MNIST_QUERY" ]]; then
  echo "=== LSH MNIST ==="
  # w sweep, L=5, k=4
  for W in 1000 2000 3000 3500 4000 5000 6000; do
    run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L5_k4_w${W}_N5.txt" -type mnist -lsh -k 4 -L 5 -w $W -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  done
  # L=10, k=4, w=3500
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L10_k4_w3500_N5.txt" -type mnist -lsh -k 4 -L 10 -w 3500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  # L=15, k=4, w=3500 (N=5,10)
  for N in 5 10; do
    run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L15_k4_w3500_N${N}.txt" -type mnist -lsh -k 4 -L 15 -w 3500 -N ${N} -R 200 -range true -qmax $QMAX -auto_w false
  done
  # L=15, k=3, w=3500
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L15_k3_w3500_N5.txt" -type mnist -lsh -k 3 -L 15 -w 3500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  # L=30, k=4,6, w=3500 with N=5,10
  for K in 4 6; do
    for N in 5 10; do
      run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/lsh/mnist_L30_k${K}_w3500_N${N}.txt" -type mnist -lsh -k ${K} -L 30 -w 3500 -N ${N} -R 200 -range true -qmax $QMAX -auto_w false
    done
  done
else
  echo "[WARN] MNIST files not found: $MNIST_TRAIN / $MNIST_QUERY — skipping LSH MNIST"
fi

########################################
# LSH — SIFT (from report)
########################################
if [[ -f "$SIFT_TRAIN" && -f "$SIFT_QUERY" ]]; then
  echo "=== LSH SIFT ==="
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L20_k3_w100_N5.txt" -type sift -lsh -k 3 -L 20 -w 100 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L20_k3_w300_N5.txt" -type sift -lsh -k 3 -L 20 -w 300 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L20_k5_w300_N5.txt" -type sift -lsh -k 5 -L 20 -w 300 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L20_k5_w500_N5.txt" -type sift -lsh -k 5 -L 20 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L30_k5_w500_N5.txt" -type sift -lsh -k 5 -L 30 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L50_k5_w500_N5.txt" -type sift -lsh -k 5 -L 50 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L50_k7_w500_N5.txt" -type sift -lsh -k 7 -L 50 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L50_k7_w600_N5.txt" -type sift -lsh -k 7 -L 50 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  # N=10 variants for selected lines
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L20_k5_w500_N10.txt" -type sift -lsh -k 5 -L 20 -w 500 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L50_k7_w500_N10.txt" -type sift -lsh -k 7 -L 50 -w 500 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/lsh/sift_L50_k7_w600_N10.txt" -type sift -lsh -k 7 -L 50 -w 600 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
else
  echo "[WARN] SIFT files not found: $SIFT_TRAIN / $SIFT_QUERY — skipping LSH SIFT"
fi

########################################
# Hypercube — MNIST (from report)
########################################
if [[ -f "$MNIST_TRAIN" && -f "$MNIST_QUERY" ]]; then
  echo "=== Hypercube MNIST ==="
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M1000_p100_w3500_N5.txt" -type mnist -hypercube -kproj 14 -M 1000 -probes 100 -w 3500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M2000_p1000_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 2000 -probes 1000 -w 2500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M5000_p2000_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 5000 -probes 2000 -w 2500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M10000_p1000_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 10000 -probes 1000 -w 2500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M15000_p3000_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 15000 -probes 3000 -w 2500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M20000_p3000_w3000_N5.txt" -type mnist -hypercube -kproj 14 -M 20000 -probes 3000 -w 3000 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k15_M20000_p10000_w2500_N5.txt" -type mnist -hypercube -kproj 15 -M 20000 -probes 10000 -w 2500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M20000_p5500_w2500_N5.txt" -type mnist -hypercube -kproj 14 -M 20000 -probes 5500 -w 2500 -N 5 -R 200 -range true -qmax $QMAX -auto_w false
  # N=10 variants
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M20000_p3000_w3000_N10.txt" -type mnist -hypercube -kproj 14 -M 20000 -probes 3000 -w 3000 -N 10 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k15_M20000_p10000_w2500_N10.txt" -type mnist -hypercube -kproj 15 -M 20000 -probes 10000 -w 2500 -N 10 -R 200 -range true -qmax $QMAX -auto_w false
  run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/hypercube/mnist_k14_M20000_p5500_w2500_N10.txt" -type mnist -hypercube -kproj 14 -M 20000 -probes 5500 -w 2500 -N 10 -R 200 -range true -qmax $QMAX -auto_w false
else
  echo "[WARN] MNIST files not found — skipping Hypercube MNIST"
fi

########################################
# Hypercube — SIFT (from report)
########################################
if [[ -f "$SIFT_TRAIN" && -f "$SIFT_QUERY" ]]; then
  echo "=== Hypercube SIFT ==="
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k14_M200000_p8000_w600_N5.txt" -type sift -hypercube -kproj 14 -M 200000 -probes 8000 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M200000_p10000_w600_N5.txt" -type sift -hypercube -kproj 15 -M 200000 -probes 10000 -w 600 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M200000_p10000_w500_N5.txt" -type sift -hypercube -kproj 15 -M 200000 -probes 10000 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M200000_p10000_w400_N5.txt" -type sift -hypercube -kproj 15 -M 200000 -probes 10000 -w 400 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M250000_p10000_w500_N5.txt" -type sift -hypercube -kproj 15 -M 250000 -probes 10000 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k14_M250000_p10000_w500_N5.txt" -type sift -hypercube -kproj 14 -M 250000 -probes 10000 -w 500 -N 5 -R 300 -range true -qmax $QMAX -auto_w false
  # N=10 variants
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M200000_p10000_w500_N10.txt" -type sift -hypercube -kproj 15 -M 200000 -probes 10000 -w 500 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k15_M250000_p10000_w500_N10.txt" -type sift -hypercube -kproj 15 -M 250000 -probes 10000 -w 500 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/hypercube/sift_k14_M250000_p10000_w500_N10.txt" -type sift -hypercube -kproj 14 -M 250000 -probes 10000 -w 500 -N 10 -R 300 -range true -qmax $QMAX -auto_w false
else
  echo "[WARN] SIFT files not found — skipping Hypercube SIFT"
fi

########################################
# IVFFlat — MNIST (from message.txt/report)
########################################
if [[ -f "$MNIST_TRAIN" && -f "$MNIST_QUERY" ]]; then
  echo "=== IVFFlat MNIST ==="
  # From silhouette selection: k=37, nprobe sweep
  for NPROBE in 1 2 3 4 5 6 7 8 9 10 15; do
   run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/ivfflat/mnist_k37_p${NPROBE}_N5.txt" -type mnist -ivfflat -kclusters 37 -nprobe ${NPROBE} -N 5 -R 200 -range true -qmax $QMAX
  done
  # N=5,10 for nprobe=5
  for N in 5 10; do
   run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/ivfflat/mnist_k37_p5_N${N}.txt" -type mnist -ivfflat -kclusters 37 -nprobe 5 -N ${N} -R 200 -range true -qmax $QMAX
  done
else
  echo "[WARN] MNIST files not found — skipping IVFFlat MNIST"
fi

########################################
# IVFFlat — SIFT (from report)
########################################
if [[ -f "$SIFT_TRAIN" && -f "$SIFT_QUERY" ]]; then
  echo "=== IVFFlat SIFT ==="
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k50_p1_N5.txt" -type sift -ivfflat -kclusters 50 -nprobe 1 -N 5 -R 300 -range true -qmax $QMAX
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k50_p10_N5.txt" -type sift -ivfflat -kclusters 50 -nprobe 10 -N 5 -R 300 -range true -qmax $QMAX
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k250_p10_N5.txt" -type sift -ivfflat -kclusters 250 -nprobe 10 -N 5 -R 300 -range true -qmax $QMAX
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k250_p25_N5.txt" -type sift -ivfflat -kclusters 250 -nprobe 25 -N 5 -R 300 -range true -qmax $QMAX
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k500_p1_N5.txt" -type sift -ivfflat -kclusters 500 -nprobe 1 -N 5 -R 300 -range true -qmax $QMAX
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k500_p10_N5.txt" -type sift -ivfflat -kclusters 500 -nprobe 10 -N 5 -R 300 -range true -qmax $QMAX
  run ./search -d "$SIFT_TRAIN" -q "$SIFT_QUERY" -o "${RESULTS_DIR}/ivfflat/sift_k500_p50_N5.txt" -type sift -ivfflat -kclusters 500 -nprobe 50 -N 5 -R 300 -range true -qmax $QMAX
else
  echo "[WARN] SIFT files not found — skipping IVFFlat SIFT"
fi

########################################
# IVFPQ — MNIST (from report)
########################################
if [[ -f "$MNIST_TRAIN" && -f "$MNIST_QUERY" ]]; then
  echo "=== IVFPQ MNIST ==="
  # k=37, nprobe sweep for M/nbits combinations
  declare -a COMBOS=(
    "37 1 16 8" "37 3 16 8" "37 5 16 8" "37 10 16 8"
    "50 5 16 8" "50 10 16 8" "50 5 16 7" "50 5 16 5" "50 5 16 9" "50 5 16 10"
    "50 5 56 8" "50 5 8 8" "50 10 56 10"
  )
  for entry in "${COMBOS[@]}"; do
    read KCL NPRO M NB <<<"$entry"
    run ./search -d "$MNIST_TRAIN" -q "$MNIST_QUERY" -o "${RESULTS_DIR}/ivfpq/mnist_k${KCL}_p${NPRO}_M${M}_nbits${NB}_N5.txt" -type mnist -ivfpq -kclusters ${KCL} -nprobe ${NPRO} -M ${M} -nbits ${NB} -N 5 -R 200 -range true -qmax $QMAX
  done
else
  echo "[WARN] MNIST files not found — skipping IVFPQ MNIST"
fi

echo "All requested tests (that had available datasets) have been launched."
