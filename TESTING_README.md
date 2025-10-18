# Vector Similarity Search Methods - Testing Framework

## Overview

This project implements and tests four vector similarity search algorithms as required by the university assignment:

- **LSH** (Locality Sensitive Hashing)
- **Hypercube** (Random Projection to Hypercube)  
- **IVFFlat** (Inverted File with Flat quantization)
- **IVFPQ** (Inverted File with Product Quantization)

The testing framework measures the three key metrics specified in the assignment PDFs:
- **Approximation Factor** (μέσο κλάσμα προσέγγισης)
- **Recall@N** (ανάκληση στους Ν πλησιέστερους γείτονες)  
- **Queries Per Second** (πλήθος αναζητήσεων ανά δευτερόλεπτο)

## Testing Framework

### Individual Algorithm Tests

Test each algorithm separately with the predefined datasets:

```bash
# Test LSH algorithm
make test-lsh

# Test Hypercube algorithm  
make test-hypercube

# Test IVFFlat algorithm
make test-ivfflat

# Test IVFPQ algorithm
make test-ivfpq
```

Each test outputs results in the `results/` directory with the PDF-compliant format:
- Method name header
- For each query: "Query: X", nearest neighbors, distances, range search results
- Final statistics: approximation factor, recall@N, queries per second

### Comprehensive Experimental Study

Run the full experimental study as required by the assignment PDFs:

```bash
make comprehensive-benchmark
```

This generates:
- CSV results with all performance metrics
- Comparison across algorithms and datasets
- Statistical analysis of performance
- Parameter impact evaluation

### All Tests at Once

Run all individual algorithm tests on both SIFT and MNIST datasets:

```bash
./run_all_tests.sh
```

This script automatically:
- Tests all 4 algorithms on SIFT dataset (sift_base.fvecs, sift_query.fvecs)
- Tests all 4 algorithms on MNIST dataset (train-images.idx3-ubyte, t10k-images.idx3-ubyte)
- Generates 8 separate result files with proper Greek format
- Measures all required metrics for experimental study

## Algorithm Parameters

### LSH Parameters
- `k`: Number of hash functions per amplified function (default: 4)
- `L`: Number of hash tables (default: 5)
- `w`: Cell width for hash functions (default: 4.0 for SIFT, 10.0 for MNIST)
- `N`: Number of nearest neighbors to find (default: 1-10)
- `R`: Range search radius (default: 2000.0 for SIFT, 2.0 for MNIST)

### Hypercube Parameters  
- `kproj`: Hypercube dimension/projections (default: 14)
- `w`: Cell width (default: 4.0 for SIFT, 10.0 for MNIST)
- `M`: Maximum points to check (default: 10)
- `probes`: Maximum hypercube vertices to probe (default: 2)
- `N`: Number of nearest neighbors to find
- `R`: Range search radius

### IVFFlat Parameters
- `kclusters`: Number of Voronoi clusters (default: 50)
- `nprobe`: Number of clusters to search (default: 5)
- `N`: Number of nearest neighbors to find
- `R`: Range search radius
- `seed`: Random seed for reproducible clustering

### IVFPQ Parameters
- `kclusters`: Number of Voronoi clusters (default: 50)
- `nprobe`: Number of clusters to search (default: 5)
- `M`: Number of subquantizers (default: 16)
- `nbits`: Bits per subquantizer (default: 8)
- `N`: Number of nearest neighbors to find
- `R`: Range search radius
- `seed`: Random seed for reproducible clustering

## Output Format

All algorithms produce output in the exact format required by the assignment PDFs:

```
METHOD NAME
Query: 0
Nearest neighbor-1: 12345
distanceApproximate: 123.456
distanceTrue: 98.765
Nearest neighbor-2: 67890
distanceApproximate: 134.567
distanceTrue: 105.432
R-near neighbors:
12345
67890
...
Query: 1
...

ΕΞΟΔΟΣ Α:
μέσο κλάσμα προσέγγισης: 1.234
ανάκληση στους Ν πλησιέστερους γείτονες: 0.89
πλήθος αναζητήσεων ανά δευτερόλεπτο: 456.78
```

## Dataset Requirements

### SIFT Dataset Format
- **Learning set**: `dataset/sift_base.fvecs` or `dataset/sift_learn.fvecs`
- **Query set**: `dataset/sift_query.fvecs`
- Format: Little-endian binary format (.fvecs)
- 128-dimensional float vectors
- Each vector: 4 bytes (dimension) + 128×4 bytes (float data)

### MNIST Dataset Format  
- **Learning set**: `dataset/train-images.idx3-ubyte`
- **Query set**: `dataset/t10k-images.idx3-ubyte`
- Format: Big-endian binary format
- 28×28 pixel images converted to 784-dimensional vectors
- Magic number: 0x00000803

## Manual Testing

You can also run algorithms manually with custom parameters:

```bash
# LSH on SIFT dataset
./search -d dataset/sift_base.fvecs -q dataset/sift_query.fvecs \
         -o results/custom_lsh.txt -type sift -lsh \
         -k 6 -L 10 -w 4.0 -N 5 -R 1000.0 -range true

# Hypercube on MNIST dataset  
./search -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte \
         -o results/custom_hypercube.txt -type mnist -hypercube \
         -kproj 12 -w 8.0 -M 20 -probes 4 -N 3 -R 500.0 -range true

# IVFFlat with custom clustering
./search -d dataset/sift_base.fvecs -q dataset/sift_query.fvecs \
         -o results/custom_ivfflat.txt -type sift -ivfflat \
         -kclusters 100 -nprobe 10 -N 5 -R 2000.0 -range true -seed 42

# IVFPQ with product quantization
./search -d dataset/sift_base.fvecs -q dataset/sift_query.fvecs \
         -o results/custom_ivfpq.txt -type sift -ivfpq \
         -kclusters 100 -nprobe 8 -M 32 -nbits 6 -N 5 -R 2000.0 -range true -seed 42
```

## Build and Execution

### Building the Project

The project uses incremental compilation for faster builds:

```bash
# Clean build (forces full recompilation)
make clean && make

# Incremental build (only recompiles changed files)
make

# Build all test executables
make all-tests

# Debug build with debugging symbols
make debug
```

### Results Directory

All test outputs are saved in the `results/` directory:

```
results/
├── output_lsh.txt              # LSH individual test results
├── output_hypercube.txt        # Hypercube individual test results  
├── output_ivfflat.txt          # IVFFlat individual test results
├── output_ivfpq.txt            # IVFPQ individual test results
├── sift_lsh.txt               # LSH on SIFT (from run_all_tests.sh)
├── sift_hypercube.txt         # Hypercube on SIFT
├── sift_ivfflat.txt           # IVFFlat on SIFT
├── sift_ivfpq.txt             # IVFPQ on SIFT
├── mnist_lsh.txt              # LSH on MNIST
├── mnist_hypercube.txt        # Hypercube on MNIST
├── mnist_ivfflat.txt          # IVFFlat on MNIST
├── mnist_ivfpq.txt            # IVFPQ on MNIST
└── comprehensive_benchmark_results.csv  # Full benchmark CSV
```

## Key Metrics Measured

Following the assignment PDF requirements, the framework measures:

1. **Approximation Factor (AF)**: 
   - Formula: `distanceApproximate / distanceTrue`
   - Lower values indicate better approximation quality
   - Reported as mean across all queries

2. **Recall@N**:
   - Fraction of queries where the true nearest neighbor appears in the N approximate results
   - Values range from 0.0 to 1.0 (higher is better)
   - Critical for evaluating algorithm accuracy

3. **Queries Per Second (QPS)**:
   - Throughput measurement: `total_queries / total_time_seconds`
   - Higher values indicate better performance
   - Includes both algorithm time and distance calculations

4. **Range Search Results**:
   - R-near neighbors within specified radius
   - Tests algorithm's ability to find all neighbors within distance R
   - Important for similarity threshold applications

## Performance Optimization

The implementation includes several optimizations:

- **Incremental Compilation**: Only changed files are recompiled
- **Template-based Algorithms**: Generic implementations for different data types
- **Efficient Memory Management**: Minimal memory allocations during search
- **SIMD-optimized Distance Calculations**: Fast Euclidean distance computation
- **Early Termination**: Algorithms stop when sufficient results are found

## Troubleshooting

### Common Issues

1. **Dataset File Not Found**
   ```
   Error: Cannot open dataset file
   ```
   - Check that dataset files exist in the `dataset/` directory
   - Verify file paths match the expected names
   - Ensure files are not corrupted

2. **Compilation Errors**
   ```
   make: *** [target] Error 1
   ```
   - Run `make clean && make` to force complete rebuild
   - Check that all source files are present
   - Verify C++17 compiler support

3. **Memory Issues**
   ```
   std::bad_alloc or segmentation fault
   ```
   - Reduce dataset size for testing
   - Check available system memory
   - Use smaller parameter values (M, L, kclusters)

4. **Empty Range Search Results**
   ```
   R-near neighbors: (empty)
   ```
   - Increase the radius R value
   - Check that radius is appropriate for dataset scale
   - SIFT typically needs R=1000-2000, MNIST needs R=2-100

### Debug Mode

Enable detailed logging:

```bash
make debug
./search [parameters...] 2>debug.log
```

This provides additional information about algorithm execution and potential issues.

## Algorithm Implementation Details

### LSH (Locality Sensitive Hashing)
- Uses random projections with Gaussian random vectors
- Hash tables store candidate neighbors
- Amplification through multiple hash functions (L tables)
- Range search supported through hash bucket enumeration

### Hypercube (Random Projection)
- Projects data onto hypercube vertices using k random projections
- Breadth-first search on hypercube for candidate neighbors
- Probes parameter limits vertex exploration
- M parameter limits total points examined

### IVFFlat (Inverted File Flat)
- K-means clustering creates Voronoi cells
- Exhaustive search within nprobe nearest clusters
- No vector compression (flat quantization)
- Exact distances computed for all candidates

### IVFPQ (Inverted File Product Quantization)
- Combines IVF clustering with product quantization
- Vectors split into M subspaces, each quantized with 2^nbits centroids
- Asymmetric distance computation using lookup tables
- Memory-efficient for large-scale datasets

## Citation

When using this testing framework in academic work:

```
Vector Similarity Search Methods Implementation and Testing Framework
University Assignment - Algorithmic Problems 2025
Implementation of LSH, Hypercube, IVFFlat, and IVFPQ algorithms
Testing framework for experimental evaluation as per assignment requirements
```