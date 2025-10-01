# Test configuration
cmake_minimum_required(VERSION 3.22)

# Create test executable
add_executable(test1 test1.cpp)

# Link against project libraries
target_link_libraries(test1
  PRIVATE
    interface_core
    compare_algo
    hypercube_core
    lsh_core
    visualization_core
    # Header-only libraries
    ivfflat_core
    ivfpq_core
    kmeans_core
    mnist_processing
    sift_processing
    metrics_core
    math_utils
)

# Include directories
target_include_directories(test1 PRIVATE ${COMMON_INCLUDE_DIRS})

# Add test to CTest
add_test(NAME basic_test COMMAND test1)