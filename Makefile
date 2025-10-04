# Makefile for Vector Similarity Search Methods
# C++17 standard required for proper functionality

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -DNDEBUG -march=native
DEBUG_FLAGS = -std=c++17 -Wall -Wextra -g -DDEBUG
INCLUDES = -I.

# Source directories
SRC_DIR = .
ALGORITHM_DIR = algorithms
DATAPROC_DIR = dataprocessing
UTILS_DIR = utils
METRICS_DIR = metrics
TESTS_DIR = tests
VIS_DIR = utils/visualization

# Object directory
OBJ_DIR = obj

# Target executables
TARGET = search
QUICK_TEST = quick_test
COMPREHENSIVE_TEST = comprehensive_test
BENCHMARK_SUITE = benchmark_suite

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
QUICK_TEST_SRC = $(TESTS_DIR)/quick_test.cpp
COMPREHENSIVE_TEST_SRC = $(TESTS_DIR)/comprehensive_test.cpp
BENCHMARK_SUITE_SRC = $(TESTS_DIR)/benchmark_suite.cpp

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
QUICK_TEST_OBJECTS = $(QUICK_TEST_SRC:%.cpp=$(OBJ_DIR)/%.o) $(LIB_OBJECTS)
COMPREHENSIVE_TEST_OBJECTS = $(COMPREHENSIVE_TEST_SRC:%.cpp=$(OBJ_DIR)/%.o) $(LIB_OBJECTS)
BENCHMARK_SUITE_OBJECTS = $(BENCHMARK_SUITE_SRC:%.cpp=$(OBJ_DIR)/%.o) $(LIB_OBJECTS)

# Create object directory structure
$(shell mkdir -p $(OBJ_DIR))
$(shell mkdir -p $(OBJ_DIR)/src)
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
$(shell mkdir -p $(OBJ_DIR)/$(TESTS_DIR))
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
$(shell mkdir -p $(OBJ_DIR)/$(METRICS_DIR))

# Default target
all: $(TARGET)

# All targets including tests
all-tests: $(TARGET) $(QUICK_TEST) $(COMPREHENSIVE_TEST) $(BENCHMARK_SUITE)

# Release build
$(TARGET): $(MAIN_OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^
	@echo "Build completed successfully!"

# Test executables
$(QUICK_TEST): $(QUICK_TEST_OBJECTS)
	@echo "Linking $(QUICK_TEST)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

$(COMPREHENSIVE_TEST): $(COMPREHENSIVE_TEST_OBJECTS)
	@echo "Linking $(COMPREHENSIVE_TEST)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

$(BENCHMARK_SUITE): $(BENCHMARK_SUITE_OBJECTS)
	@echo "Linking $(BENCHMARK_SUITE)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Debug build
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGET)

# Compile source files to object files
$(OBJ_DIR)/%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET) $(QUICK_TEST) $(COMPREHENSIVE_TEST) $(BENCHMARK_SUITE)
	rm -rf tests/*.csv
	rm -rf visualization/*/
	@echo "Clean completed!"

# Clean and rebuild
rebuild: clean all

# Install target (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Test targets
test-lsh: $(TARGET)
	./$(TARGET) -d dataset/input.dat -q dataset/query.dat -o output_lsh.txt -type sift -lsh -k 4 -L 5 -w 4.0 -N 1 -R 2.0

test-hypercube: $(TARGET)
	./$(TARGET) -d dataset/input.dat -q dataset/query.dat -o output_hypercube.txt -type sift -hypercube -kproj 14 -w 4.0 -M 10 -probes 2 -N 1 -R 2.0

test-ivfflat: $(TARGET)
	./$(TARGET) -d dataset/input.dat -q dataset/query.dat -o output_ivfflat.txt -type sift -ivfflat -kclusters 50 -nprobe 5 -N 1 -R 2.0

test-ivfpq: $(TARGET)
	./$(TARGET) -d dataset/input.dat -q dataset/query.dat -o output_ivfpq.txt -type sift -ivfpq -kclusters 50 -nprobe 5 -M 16 -nbits 8 -N 1 -R 2.0

# Quick development test with synthetic data
quick-test: $(QUICK_TEST)
	@echo "Running quick algorithm comparison test..."
	@mkdir -p tests visualization
	./$(QUICK_TEST)

# Comprehensive test suite
comprehensive-test: $(COMPREHENSIVE_TEST)
	@echo "Running comprehensive algorithm tests..."
	@mkdir -p tests visualization
	./$(COMPREHENSIVE_TEST)

# Full benchmark suite
benchmark: $(BENCHMARK_SUITE)
	@echo "Running full benchmark suite..."
	@mkdir -p tests visualization Dataset
	./$(BENCHMARK_SUITE)

# Generate documentation
docs:
	@echo "Generating documentation..."
	doxygen Doxyfile 2>/dev/null || echo "Doxygen not available, skipping documentation generation"

# Help target
help:
	@echo "Available targets:"
	@echo "  all                 - Build main search executable"
	@echo "  all-tests          - Build all executables including tests"
	@echo "  debug              - Build with debug flags"
	@echo "  clean              - Remove build artifacts and results"
	@echo "  rebuild            - Clean and build"
	@echo "  quick-test         - Run quick test with synthetic data"
	@echo "  comprehensive-test - Run full algorithm comparison"
	@echo "  benchmark          - Run complete benchmark suite"
	@echo "  test-lsh           - Test LSH algorithm"
	@echo "  test-hypercube     - Test Hypercube algorithm"
	@echo "  test-ivfflat       - Test IVFFlat algorithm"
	@echo "  test-ivfpq         - Test IVFPQ algorithm"
	@echo "  docs               - Generate documentation"
	@echo "  install            - Install to /usr/local/bin"
# Phony targets
.PHONY: all all-tests debug clean rebuild install quick-test comprehensive-test benchmark docs help
.PHONY: test-lsh test-hypercube test-ivfflat test-ivfpq

# Dependencies (basic)
$(OBJ_DIR)/main.o: main.cpp $(UTILS_DIR)/interface/Interface.h
$(OBJ_DIR)/$(INTERFACE_SRC:%.cpp=%.o): $(INTERFACE_SRC) $(UTILS_DIR)/interface/Interface.h
$(OBJ_DIR)/$(LSH_SRC:%.cpp=%.o): $(LSH_SRC) $(ALGORITHM_DIR)/LSH/LSH.h
$(OBJ_DIR)/$(HYPERCUBE_SRC:%.cpp=%.o): $(HYPERCUBE_SRC) $(ALGORITHM_DIR)/hypercube/Hypercube.h
$(OBJ_DIR)/$(IVFFLAT_SRC:%.cpp=%.o): $(IVFFLAT_SRC) $(ALGORITHM_DIR)/IVFFlat/IVFFlat.h
$(OBJ_DIR)/$(IVFPQ_SRC:%.cpp=%.o): $(IVFPQ_SRC) $(ALGORITHM_DIR)/IVFPQ/IVFPQ.h
