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

# Source files
MAIN_SRC = src/main.cpp

INTERFACE_SRC = $(UTILS_DIR)/interface/Interface.cpp $(UTILS_DIR)/interface/print.cpp
UTILS_SRC = $(UTILS_DIR)/helpers/Utils.cpp
MATH_SRC = $(UTILS_DIR)/math/math.cpp $(UTILS_DIR)/math/Gaussian.cpp $(UTILS_DIR)/math/Random.cpp

SIFT_SRC = $(DATAPROC_DIR)/SIFT/SIFT.cpp
MNIST_SRC = $(DATAPROC_DIR)/MNIST/MNIST.cpp

LSH_SRC = $(ALGORITHM_DIR)/LSH/LSH.cpp
HYPERCUBE_SRC = $(ALGORITHM_DIR)/hypercube/Hypercube.cpp
KMEANS_SRC = $(ALGORITHM_DIR)/Kmeans/kmeans.cpp
IVFFLAT_SRC = $(ALGORITHM_DIR)/IVFFlat/IVFFlat.cpp
IVFPQ_SRC = $(ALGORITHM_DIR)/IVFPQ/IVFPQ.cpp
COMPARE_SRC = $(ALGORITHM_DIR)/compare/compareAlgo.cpp

METRICS_SRC = $(METRICS_DIR)/metrics.cpp
VIS_SRC = $(VIS_DIR)/visualizer.cpp

# Test sources
COMPREHENSIVE_BENCHMARK_SRC = src/comprehensive_benchmark.cpp
VISUAL_BENCHMARK_SRC = src/visual_benchmark.cpp

# Common library sources (excluding main files)
LIB_SOURCES = $(INTERFACE_SRC) $(UTILS_SRC) $(MATH_SRC) \
              $(SIFT_SRC) $(MNIST_SRC) $(LSH_SRC) $(HYPERCUBE_SRC) \
              $(KMEANS_SRC) $(IVFFLAT_SRC) $(IVFPQ_SRC) $(COMPARE_SRC) \
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
$(shell mkdir -p $(OBJ_DIR)/$(ALGORITHM_DIR)/compare)
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

# Install target (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Test targets
test-lsh: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_lsh.txt -type sift -lsh -k 4 -L 5 -w 4.0 -N 1 -R 2.0

test-hypercube: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_hypercube.txt -type sift -hypercube -kproj 14 -w 4.0 -M 10 -probes 2 -N 1 -R 2.0

test-ivfflat: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_ivfflat.txt -type sift -ivfflat -kclusters 50 -nprobe 5 -N 1 -R 2.0

test-ivfpq: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_ivfpq.txt -type sift -ivfpq -kclusters 50 -nprobe 5 -M 16 -nbits 8 -N 1 -R 2.0


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

# Generate documentation
docs:
	@echo "Generating documentation..."
	doxygen Doxyfile 2>/dev/null || echo "Doxygen not available, skipping documentation generation"

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
	@echo "  test-lsh           - Test LSH algorithm"
	@echo "  test-hypercube     - Test Hypercube algorithm"
	@echo "  test-ivfflat       - Test IVFFlat algorithm"
	@echo "  test-ivfpq         - Test IVFPQ algorithm"
	@echo "  docs               - Generate documentation"
	@echo "  install            - Install to /usr/local/bin"
	@echo ""
	@echo "NEW: visual-benchmark provides comprehensive analysis with:"
	@echo "  - Parameter sweep testing for all algorithms"
	@echo "  - Automatic plot generation (bar charts, trade-off analysis)"
	@echo "  - Parameter impact analysis"
	@echo "  - Interactive HTML report with embedded visualizations"
	@echo "  - Results saved to results/visualization/ directory"
	@echo ""
	@echo "Note: Makefile uses incremental compilation - only changed files are recompiled"
	@echo "Use 'rebuild' to force full recompilation if needed"
# Phony targets
.PHONY: all all-tests debug clean rebuild install comprehensive-benchmark visual-benchmark docs help
.PHONY: test-lsh test-hypercube test-ivfflat test-ivfpq

# Note: This Makefile uses automatic dependency generation for incremental compilation
# Only changed source files and their dependencies will be recompiled

# Dependencies (basic)
$(OBJ_DIR)/main.o: main.cpp $(UTILS_DIR)/interface/Interface.h
$(OBJ_DIR)/$(INTERFACE_SRC:%.cpp=%.o): $(INTERFACE_SRC) $(UTILS_DIR)/interface/Interface.h
$(OBJ_DIR)/$(LSH_SRC:%.cpp=%.o): $(LSH_SRC) $(ALGORITHM_DIR)/LSH/LSH.h
$(OBJ_DIR)/$(HYPERCUBE_SRC:%.cpp=%.o): $(HYPERCUBE_SRC) $(ALGORITHM_DIR)/hypercube/Hypercube.h
$(OBJ_DIR)/$(IVFFLAT_SRC:%.cpp=%.o): $(IVFFLAT_SRC) $(ALGORITHM_DIR)/IVFFlat/IVFFlat.h
$(OBJ_DIR)/$(IVFPQ_SRC:%.cpp=%.o): $(IVFPQ_SRC) $(ALGORITHM_DIR)/IVFPQ/IVFPQ.h
