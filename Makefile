# Makefile for Vector Similarity Search Methods
# C++17 standard required for proper functionality

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -DNDEBUG -march=native
DEBUG_FLAGS = -std=c++17 -Wall -Wextra -g -DDEBUG
INCLUDES = -I.

# Dependency generation flags
DEPFLAGS = -MMD -MP

# Source directories
SRC_DIR = .
ALGORITHM_DIR = algorithms
DATAPROC_DIR = dataprocessing
UTILS_DIR = utils
METRICS_DIR = metrics
VIS_DIR = utils/visualization

# Object directory (created automatically during compilation)
OBJ_DIR = obj

# Target executables
TARGET = search
COMPREHENSIVE_BENCHMARK = comprehensive_benchmark
VISUAL_BENCHMARK = visual_benchmark

# Max queries to process (override with `make QMAX=1000 <target>`)
QMAX ?= 100

# Source files
MAIN_SRC = src/main.cpp

INTERFACE_SRC = $(UTILS_DIR)/interface/Interface.cpp
UTILS_SRC = $(UTILS_DIR)/helpers/Utils.cpp
MATH_SRC = $(UTILS_DIR)/math/math.cpp

SIFT_SRC = $(DATAPROC_DIR)/SIFT/SIFT.cpp
MNIST_SRC = $(DATAPROC_DIR)/MNIST/MNIST.cpp

LSH_SRC = $(ALGORITHM_DIR)/LSH/LSH.cpp
HYPERCUBE_SRC = $(ALGORITHM_DIR)/hypercube/Hypercube.cpp
KMEANS_SRC = $(ALGORITHM_DIR)/Kmeans/kmeans.cpp
IVFFLAT_SRC = $(ALGORITHM_DIR)/IVFFlat/IVFFlat.cpp
IVFPQ_SRC = $(ALGORITHM_DIR)/IVFPQ/IVFPQ.cpp
BRUTEFORCE_SRC = $(ALGORITHM_DIR)/BruteForce/BruteForce.cpp

METRICS_SRC = $(METRICS_DIR)/metrics.cpp
VIS_SRC = $(VIS_DIR)/visualizer.cpp

# Test sources
COMPREHENSIVE_BENCHMARK_SRC = src/comprehensive_benchmark.cpp
VISUAL_BENCHMARK_SRC = src/visual_benchmark.cpp

# Common library sources (excluding main files)
LIB_SOURCES = $(INTERFACE_SRC) $(UTILS_SRC) $(MATH_SRC) \
              $(SIFT_SRC) $(MNIST_SRC) $(LSH_SRC) $(HYPERCUBE_SRC) \
              $(KMEANS_SRC) $(IVFFLAT_SRC) $(IVFPQ_SRC) $(BRUTEFORCE_SRC) \
              $(METRICS_SRC) $(VIS_SRC)

# Main program sources
MAIN_SOURCES = $(MAIN_SRC) $(LIB_SOURCES)

# Object files
MAIN_OBJECTS = $(MAIN_SOURCES:%.cpp=$(OBJ_DIR)/%.o)
LIB_OBJECTS = $(LIB_SOURCES:%.cpp=$(OBJ_DIR)/%.o)
COMPREHENSIVE_BENCHMARK_OBJECTS = $(COMPREHENSIVE_BENCHMARK_SRC:%.cpp=$(OBJ_DIR)/%.o) $(LIB_OBJECTS)
VISUAL_BENCHMARK_OBJECTS = $(VISUAL_BENCHMARK_SRC:%.cpp=$(OBJ_DIR)/%.o) $(LIB_OBJECTS)

# Dependency files
DEPS = $(MAIN_OBJECTS:.o=.d) $(LIB_OBJECTS:.o=.d) \
       $(COMPREHENSIVE_BENCHMARK_SRC:%.cpp=$(OBJ_DIR)/%.d) \
       $(VISUAL_BENCHMARK_SRC:%.cpp=$(OBJ_DIR)/%.d)

# Dependencies (if they exist)
-include $(DEPS)

# Create object directory structure (done automatically in compilation rule)
# No manual directory creation needed
$(shell mkdir -p $(OBJ_DIR)/$(UTILS_DIR)/interface)
$(shell mkdir -p $(OBJ_DIR)/$(UTILS_DIR)/helpers)
$(shell mkdir -p $(OBJ_DIR)/$(UTILS_DIR)/math)
$(shell mkdir -p $(OBJ_DIR)/$(DATAPROC_DIR)/SIFT)
$(shell mkdir -p $(OBJ_DIR)/$(DATAPROC_DIR)/MNIST)
$(shell mkdir -p $(OBJ_DIR)/$(ALGORITHM_DIR)/LSH)
$(shell mkdir -p $(OBJ_DIR)/$(ALGORITHM_DIR)/hypercube)
$(shell mkdir -p $(OBJ_DIR)/$(ALGORITHM_DIR)/Kmeans)
$(shell mkdir -p $(OBJ_DIR)/$(ALGORITHM_DIR)/IVFFlat)
$(shell mkdir -p $(OBJ_DIR)/$(ALGORITHM_DIR)/IVFPQ)
$(shell mkdir -p $(OBJ_DIR)/$(ALGORITHM_DIR)/BruteForce)
$(shell mkdir -p $(OBJ_DIR)/$(METRICS_DIR))
$(shell mkdir -p $(OBJ_DIR)/$(VIS_DIR))
$(shell mkdir -p $(OBJ_DIR)/src)

# Default target
all: $(TARGET)

# All targets including tests
all-tests: $(TARGET) $(COMPREHENSIVE_BENCHMARK) $(VISUAL_BENCHMARK)

# Release build
$(TARGET): $(MAIN_OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^
	@echo "Build completed successfully!"

# Test executables
$(COMPREHENSIVE_BENCHMARK): $(COMPREHENSIVE_BENCHMARK_OBJECTS)
	@echo "Linking $(COMPREHENSIVE_BENCHMARK)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

$(VISUAL_BENCHMARK): $(VISUAL_BENCHMARK_OBJECTS)
	@echo "Linking $(VISUAL_BENCHMARK)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Debug build
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGET)

# Compile source files to object files with dependency generation
$(OBJ_DIR)/%.o: %.cpp
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) -c $< -o $@

# Include dependency files (if they exist)
-include $(DEPS)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET) $(COMPREHENSIVE_BENCHMARK) $(VISUAL_BENCHMARK)
	rm -rf tests/*.csv
	rm -rf results/visualization/
	@echo "Clean completed!"

# Clean and rebuild
rebuild: clean all

# Test targets
test-lsh-sift: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_lsh_sift.txt -type sift -lsh -k 10 -L 100 -w 4.0 -N 4 -R 500.0 -range true -qmax $(QMAX)

test-lsh-mnist: $(TARGET)
	./$(TARGET) -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/output_lsh_mnist.txt -type mnist -lsh -k 10 -L 100 -w 200 -N 4 -R 200 -range true -qmax $(QMAX)

test-hypercube-sift: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_hypercube_sift.txt -type sift -hypercube -kproj 14 -w 200 -M 20000 -probes 10000 -N 4 -R 500.0 -range true -qmax $(QMAX)

test-hypercube-mnist: $(TARGET)
	./$(TARGET) -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/output_hypercube_mnist.txt -type mnist -hypercube -kproj 15 -w 200 -M 20000 -probes 10000 -N 4 -R 200 -range true -qmax $(QMAX)

test-ivfflat-sift: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_ivfflat_sift.txt -type sift -ivfflat -kclusters 50 -nprobe 5 -N 4 -R 2.0 -qmax $(QMAX)

test-ivfflat-mnist: $(TARGET)
	./$(TARGET) -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/output_ivfflat_mnist.txt -type mnist -ivfflat -kclusters 50 -nprobe 5 -N 4 -R 2000 -qmax $(QMAX)


test-ivfpq-sift: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_ivfpq_sift.txt -type sift -ivfpq -kclusters 50 -nprobe 5 -M 16 -nbits 8 -N 4 -R 2.0 -qmax $(QMAX)

test-ivfpq-mnist: $(TARGET)
	./$(TARGET) -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/output_ivfpq_mnist.txt -type mnist -ivfpq -kclusters 50 -nprobe 5 -M 16 -nbits 8 -N 4 -R 2000 -qmax $(QMAX)



# Comprehensive benchmark with experimental study metrics
comprehensive-benchmark: $(COMPREHENSIVE_BENCHMARK)
	@echo "Running comprehensive experimental study as required by PDFs..."
	@echo "Measuring: Approximation Factor, Recall@N, Queries Per Second"
	@mkdir -p results tests
	./$(COMPREHENSIVE_BENCHMARK)

# Visual benchmark with parameter sweeps and comprehensive visualization
visual-benchmark: $(VISUAL_BENCHMARK)
	@echo "Running comprehensive visual benchmark with parameter analysis..."
	@echo "Generating plots, parameter impact analysis, and HTML reports"
	@mkdir -p results/visualization
	./$(VISUAL_BENCHMARK)

# Help target
help:
	@echo "Available targets:"
	@echo "  all                 - Build main search executable (incremental)"
	@echo "  all-tests          - Build all executables including tests"
	@echo "  debug              - Build with debug flags"
	@echo "  clean              - Remove build artifacts and results"
	@echo "  rebuild            - Clean and build (forces full recompilation)"
	@echo "  comprehensive-benchmark - Run PDF-required experimental study (AF, Recall@N, QPS)"
	@echo "  visual-benchmark   - Run comprehensive visual analysis with parameter sweeps"
	@echo "  test-lsh-sift      - Test LSH algorithm with SIFT dataset"
	@echo "  test-lsh-mnist     - Test LSH algorithm with MNIST dataset"
	@echo "  test-hypercube-sift     - Test Hypercube algorithm with SIFT dataset"
	@echo "  test-hypercube-mnist     - Test Hypercube algorithm with MNIST dataset"
	@echo "  test-ivfflat-sift  - Test IVFFlat algorithm with SIFT dataset"
	@echo "  test-ivfflat-mnist - Test IVFFlat algorithm with MNIST dataset"
	@echo "  test-ivfpq-sift    - Test IVFPQ algorithm with SIFT dataset"
	@echo "  test-ivfpq-mnist   - Test IVFPQ algorithm with MNIST dataset"
	@echo "  install            - Install to /usr/local/bin"
	
# Phony targets
.PHONY: all all-tests debug clean rebuild install comprehensive-benchmark visual-benchmark docs help
.PHONY: test-lsh test-lsh-sift test-lsh-mnist test-hypercube test-hypercube-sift test-hypercube-mnist
.PHONY: test-ivfflat test-ivfflat-sift test-ivfflat-mnist test-ivfpq test-ivfpq-sift test-ivfpq-mnist

# Dependencies (basic)
$(OBJ_DIR)/main.o: main.cpp $(UTILS_DIR)/interface/Interface.h
$(OBJ_DIR)/$(INTERFACE_SRC:%.cpp=%.o): $(INTERFACE_SRC) $(UTILS_DIR)/interface/Interface.h
$(OBJ_DIR)/$(LSH_SRC:%.cpp=%.o): $(LSH_SRC) $(ALGORITHM_DIR)/LSH/LSH.h
$(OBJ_DIR)/$(HYPERCUBE_SRC:%.cpp=%.o): $(HYPERCUBE_SRC) $(ALGORITHM_DIR)/hypercube/Hypercube.h
$(OBJ_DIR)/$(IVFFLAT_SRC:%.cpp=%.o): $(IVFFLAT_SRC) $(ALGORITHM_DIR)/IVFFlat/IVFFlat.h
$(OBJ_DIR)/$(IVFPQ_SRC:%.cpp=%.o): $(IVFPQ_SRC) $(ALGORITHM_DIR)/IVFPQ/IVFPQ.h
