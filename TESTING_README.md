# Vector Similarity Search Methods - Testing and Visualization Framework

## Overview

This project implements comprehensive testing and visualization tools for comparing vector similarity search algorithms:

- **LSH** (Locality Sensitive Hashing)
- **Hypercube** (Random Projection to Hypercube)  
- **IVFFlat** (Inverted File with Flat quantization)
- **IVFPQ** (Inverted File with Product Quantization)

## Testing Framework

### Quick Testing (Development)

Run quick tests with synthetic data for development and debugging:

```bash
make quick-test
```

This generates:
- Small synthetic datasets (100-dimensional vectors)
- Parameter sweeps for each algorithm
- Performance comparisons
- Basic visualizations

### Comprehensive Testing

Full algorithm comparison with parameter sweeps:

```bash
make comprehensive-test
```

Features:
- Tests all algorithms with extensive parameter ranges
- Supports both MNIST and SIFT datasets
- Generates detailed CSV results
- Creates comprehensive visualization reports

### Benchmark Suite

Complete benchmark with optimal parameter exploration:

```bash
make benchmark
```

This runs:
- Exhaustive parameter sweeps for all algorithms
- Cross-dataset comparisons
- Memory usage analysis
- Performance optimization recommendations

## Test Parameters

### LSH Parameters
- `k`: Number of hash functions per amplified function (2-10)
- `L`: Number of hash tables (1-20)
- `w`: Cell width for hash functions (0.5-16.0)

### Hypercube Parameters  
- `k`: Hypercube dimension (8-18)
- `w`: Cell width (0.5-8.0)
- `M`: Maximum points to check (10-500)
- `probes`: Maximum vertices to probe (1-16)

### IVFFlat Parameters
- `n_clusters`: Number of clusters (10-2000)
- `n_probe`: Clusters to search (1-100)

### IVFPQ Parameters
- `n_clusters`: Number of clusters (50-2000)
- `n_probe`: Clusters to search (1-100)
- `m`: Number of subquantizers (8-64)
- `n_bits`: Bits per subquantizer (6-10)

## Visualization Framework

### Automatic Plot Generation

The visualization system automatically generates:

1. **Algorithm Comparison Plots**
   - Bar charts comparing AF, Recall@N, QPS, Query Time
   - Scatter plots for trade-off analysis

2. **Parameter Impact Analysis**
   - Individual parameter vs performance plots
   - Sensitivity analysis for each algorithm

3. **Performance Analysis**
   - Memory usage comparisons
   - Speed vs accuracy trade-offs
   - Recall vs approximation factor plots

### Generated Outputs

Results are organized in the `visualization/` directory:

```
visualization/
├── quick_test/                # Quick test results
├── mnist_plots/              # MNIST dataset analysis
├── sift_plots/               # SIFT dataset analysis
├── parameter_impact/         # Parameter sensitivity analysis
└── complete_comparison/      # Cross-dataset comparison
```

Each directory contains:
- `*.png` - Individual plot images
- `benchmark_report.html` - Interactive HTML report
- Python plotting scripts (auto-generated)

### Python Dependencies

For visualization, ensure Python 3 with these packages:

```bash
pip install matplotlib pandas seaborn numpy
```

## Usage Examples

### 1. Quick Development Test

```bash
# Build and run quick test
make quick-test

# Results in:
# - tests/quick_test_results.csv
# - visualization/quick_test/
```

### 2. MNIST Dataset Analysis

```bash
# Place MNIST files in Dataset/ directory
# mnist_train.dat, mnist_test.dat

make comprehensive-test
```

### 3. SIFT Dataset Analysis

```bash
# Place SIFT files in Dataset/ directory  
# sift_base.fvecs, sift_query.fvecs

make benchmark
```

### 4. Custom Parameter Testing

Modify test files to focus on specific parameters:

```cpp
// In tests/quick_test.cpp
std::vector<uint32_t> k_values = {4, 6, 8};  // Test specific k values
std::vector<uint32_t> L_values = {5, 10};    // Test specific L values
```

## Results Analysis

### CSV Output Format

Results are exported in CSV format with columns:
- Algorithm name
- Performance metrics (AF, Recall@N, QPS, Time, Memory)
- All parameter values used
- Individual query statistics

### HTML Reports

Interactive reports include:
- Summary statistics tables
- Embedded plots and charts
- Parameter impact analysis
- Cross-algorithm comparisons
- Best configuration recommendations

### Key Metrics

- **AF (Approximation Factor)**: approx_distance / true_distance
- **Recall@N**: Fraction of queries returning true nearest neighbor
- **QPS**: Queries per second (throughput)
- **Memory**: Memory usage in MB

## Dataset Requirements

### MNIST Format
- Big-endian binary format
- 28x28 pixel images (784 dimensions)
- Magic number: 0x00000803

### SIFT Format  
- Little-endian binary format (.fvecs)
- 128-dimensional float vectors
- Each vector: 4 bytes (dim) + 128×4 bytes (data)

## Performance Optimization

The testing framework includes optimizations:
- Template-based generic algorithms
- Efficient memory management
- Parallel-friendly design
- Incremental result processing

## Troubleshooting

### Common Issues

1. **Dataset Loading Errors**
   - Check file paths in test files
   - Verify dataset format (endianness)
   - Ensure sufficient disk space

2. **Visualization Errors**
   - Install required Python packages
   - Check Python 3 availability
   - Verify write permissions in visualization/

3. **Memory Issues**
   - Reduce dataset size for testing
   - Lower parameter ranges
   - Use smaller synthetic datasets

### Debug Mode

Build with debug information:

```bash
make debug
make quick-test
```

This enables additional logging and error checking.

## Extension Guide

### Adding New Algorithms

1. Implement algorithm class following existing patterns
2. Add benchmark method to `AlgorithmComparator`
3. Update test files to include new algorithm
4. Add visualization support

### Custom Metrics

Add new evaluation metrics by:
1. Extending `BenchmarkResult` structure
2. Implementing metric computation
3. Adding visualization support
4. Updating export functions

### Custom Datasets

Support new datasets by:
1. Adding parser to `dataprocessing/`
2. Updating `AlgorithmComparator::load_dataset()`
3. Adding dataset-specific optimizations

## Citation

When using this testing framework, please cite:

```
Vector Similarity Search Testing Framework
University Assignment Implementation
Authors: [Your Name]
Year: 2025
```