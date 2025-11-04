# Makefile for Vector Similarity Search Methods
# C++17 standard required for proper functionality

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -DNDEBUG -march=native -fopenmp
DEBUG_FLAGS = -std=c++17 -Wall -Wextra -g -DDEBUG -fopenmp
INCLUDES = -I.

# Dependency generation flags
DEPFLAGS = -MMD -MP

# Source directories
SRC_DIR = .
ALGORITHM_DIR = algorithms
DATAPROC_DIR = dataprocessing
UTILS_DIR = utils
METRICS_DIR = metrics
 

# Object directory (created automatically during compilation)
OBJ_DIR = obj

# Target executables
TARGET = search
 

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

METRICS_SRC = $(METRICS_DIR)/metrics.cpp
 

# Test sources

# Common library sources (excluding main files)
LIB_SOURCES = $(INTERFACE_SRC) $(UTILS_SRC) $(MATH_SRC) \
              $(SIFT_SRC) $(MNIST_SRC) $(LSH_SRC) $(HYPERCUBE_SRC) \
              $(KMEANS_SRC) $(IVFFLAT_SRC) $(IVFPQ_SRC) \
			  $(METRICS_SRC)

# Main program sources
MAIN_SOURCES = $(MAIN_SRC) $(LIB_SOURCES)

# Object files
MAIN_OBJECTS = $(MAIN_SOURCES:%.cpp=$(OBJ_DIR)/%.o)
LIB_OBJECTS = $(LIB_SOURCES:%.cpp=$(OBJ_DIR)/%.o)
 

# Dependency files
DEPS = $(MAIN_OBJECTS:.o=.d) $(LIB_OBJECTS:.o=.d)

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
$(shell mkdir -p $(OBJ_DIR)/$(METRICS_DIR))
$(shell mkdir -p $(OBJ_DIR)/src)

# Default target
all: $(TARGET)

# All targets including tests
all-tests: $(TARGET)

# Release build
$(TARGET): $(MAIN_OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^
	@echo "Build completed successfully!"

# (no additional test executables)

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
	rm -f $(TARGET)
	rm -rf tests/*.csv
	@echo "Clean completed!"

# Clean and rebuild
rebuild: clean all

# Test targets
test-lsh-sift: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_lsh_sift.txt -type sift -lsh -k 14 -L 5 -w 500 -N 4 -R 300.0 -range true -qmax $(QMAX)

test-lsh-mnist: $(TARGET)
	./$(TARGET) -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/output_lsh_mnist.txt -type mnist -lsh -k 10 -L 100 -w 200 -N 4 -R 200 -range true -qmax $(QMAX)

test-hypercube-sift: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_hypercube_sift.txt -type sift -hypercube -kproj 14 -w 200 -M 20000 -probes 10000 -N 4 -R 300.0 -range true -qmax $(QMAX)

test-hypercube-mnist: $(TARGET)
	./$(TARGET) -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/output_hypercube_mnist.txt -type mnist -hypercube -kproj 15 -w 200 -M 20000 -probes 10000 -N 4 -R 200 -range true -qmax $(QMAX)

test-ivfflat-sift: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_ivfflat_sift.txt -type sift -ivfflat -kclusters 50 -nprobe 5 -N 4 -R 2.0 -qmax $(QMAX)

test-ivfflat-mnist: $(TARGET)
	./$(TARGET) -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/output_ivfflat_mnist.txt -type mnist -ivfflat -kclusters 50 -nprobe 5 -N 4 -R 2000 -qmax $(QMAX)


test-ivfpq-sift: $(TARGET)
	./$(TARGET) -d dataset/sift_learn.fvecs -q dataset/sift_query.fvecs -o results/output_ivfpq_sift.txt -type sift -ivfpq -kclusters 50 -nprobe 5 -M 8 -nbits 8 -N 4 -R 2.0 -qmax $(QMAX)

test-ivfpq-mnist: $(TARGET)
	./$(TARGET) -d dataset/train-images.idx3-ubyte -q dataset/t10k-images.idx3-ubyte -o results/output_ivfpq_mnist.txt -type mnist -ivfpq -kclusters 50 -nprobe 5 -M 8 -nbits 8 -N 4 -R 2000 -qmax $(QMAX)

# Experimental Study Targets
analyze-study:
	@echo "Analyzing Experimental Study Results..."
	@python3 experimental_study/analyze_experimental_study.py
	@echo ""
	@echo "Report generated: results/REPORT_TESTS_SUMMARY.md"

experimental-study: $(TARGET)
	@echo "Running full experimental study..."
	@chmod +x experimental_study/run_experimental_study.sh
	@./experimental_study/run_experimental_study.sh

experimental-study2: $(TARGET)
	@echo "Running full experimental study2..."
	@chmod +x experimental_study/run_experimental_study2.sh
	@./experimental_study/run_experimental_study2.sh

experimental-study3: $(TARGET)
	@echo "Running full experimental study3..."
	@chmod +x experimental_study/run_experimental_study3.sh
	@./experimental_study/run_experimental_study3.sh

# Help target
help:
	@echo "Available targets:"
	@echo "  all                 - Build main search executable (incremental)"
	@echo "  all-tests          - Build all executables including tests"
	@echo "  debug              - Build with debug flags"
	@echo "  clean              - Remove build artifacts and results"
	@echo "  rebuild            - Clean and build (forces full recompilation)"
	@echo "  experimental-study - Run full experimental comparative study "
	@echo "  analyze-study      - Analyze experimental study results and generate report"
	@echo "  test-lsh-sift      - Test LSH algorithm with SIFT dataset"
	@echo "  test-lsh-mnist     - Test LSH algorithm with MNIST dataset"
	@echo "  test-hypercube-sift     - Test Hypercube algorithm with SIFT dataset"
	@echo "  test-hypercube-mnist     - Test Hypercube algorithm with MNIST dataset"
	@echo "  test-ivfflat-sift  - Test IVFFlat algorithm with SIFT dataset"
	@echo "  test-ivfflat-mnist - Test IVFFlat algorithm with MNIST dataset"
	@echo "  test-ivfpq-sift    - Test IVFPQ algorithm with SIFT dataset"
	@echo "  test-ivfpq-mnist   - Test IVFPQ algorithm with MNIST dataset"

# Phony targets
.PHONY: all all-tests debug clean rebuild install docs help
.PHONY: experimental-study experimental-study2 experimental-study3 analyze-study
.PHONY: test-lsh test-lsh-sift test-lsh-mnist test-hypercube test-hypercube-sift test-hypercube-mnist
.PHONY: test-ivfflat test-ivfflat-sift test-ivfflat-mnist test-ivfpq test-ivfpq-sift test-ivfpq-mnist

# Dependencies (basic)
$(OBJ_DIR)/src/main.o: src/main.cpp $(UTILS_DIR)/interface/Interface.h
$(OBJ_DIR)/$(INTERFACE_SRC:%.cpp=%.o): $(INTERFACE_SRC) $(UTILS_DIR)/interface/Interface.h
$(OBJ_DIR)/$(LSH_SRC:%.cpp=%.o): $(LSH_SRC) $(ALGORITHM_DIR)/LSH/LSH.h
$(OBJ_DIR)/$(HYPERCUBE_SRC:%.cpp=%.o): $(HYPERCUBE_SRC) $(ALGORITHM_DIR)/hypercube/Hypercube.h
$(OBJ_DIR)/$(IVFFLAT_SRC:%.cpp=%.o): $(IVFFLAT_SRC) $(ALGORITHM_DIR)/IVFFlat/IVFFlat.h
$(OBJ_DIR)/$(IVFPQ_SRC:%.cpp=%.o): $(IVFPQ_SRC) $(ALGORITHM_DIR)/IVFPQ/IVFPQ.h
