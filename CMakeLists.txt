cmake_minimum_required(VERSION 3.22)
project(VectorSimilaritySearch
  VERSION 0.1.0
  LANGUAGES CXX)

# Γενικές ρυθμίσεις C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # για clangd/VSCode

# Αν δεν οριστεί generator, προτίμησε Ninja (γρήγορο)
if (NOT CMAKE_GENERATOR)
  set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)
endif()

# Warnings για GCC/Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Κοινά include paths (εφόσον έχεις headers διασπαρμένους)
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(COMMON_INCLUDE_DIRS
  ${PROJECT_ROOT}
  ${PROJECT_ROOT}/algorithms/compare
  ${PROJECT_ROOT}/algorithms/hypercube
  ${PROJECT_ROOT}/algorithms/IVFFlat
  ${PROJECT_ROOT}/algorithms/IVFPQ
  ${PROJECT_ROOT}/algorithms/Kmeans
  ${PROJECT_ROOT}/algorithms/LSH
  ${PROJECT_ROOT}/dataprocessing/MNIST
  ${PROJECT_ROOT}/dataprocessing/SIFT
  ${PROJECT_ROOT}/metrics
  ${PROJECT_ROOT}/utils/visualization
  ${PROJECT_ROOT}/utils/interface
  ${PROJECT_ROOT}/utils/math
)

# Υποκατάλογοι που δημιουργούν libraries/targets
add_subdirectory(utils/visualization)
add_subdirectory(utils/interface)
add_subdirectory(utils/math)
add_subdirectory(algorithms/compare)
add_subdirectory(algorithms/hypercube)
add_subdirectory(algorithms/IVFFlat)
add_subdirectory(algorithms/LSH)
add_subdirectory(algorithms/Kmeans)
add_subdirectory(algorithms/IVFPQ)

# Header-only "βιβλιοθήκες" για ομαδοποίηση includes
add_subdirectory(dataprocessing/MNIST)
add_subdirectory(dataprocessing/SIFT)
add_subdirectory(metrics)

# Κύριο εκτελέσιμο από src/main.cpp
add_executable(vss_app src/main.cpp)
target_include_directories(vss_app PUBLIC ${COMMON_INCLUDE_DIRS})
target_link_libraries(vss_app
  PRIVATE
    interface_core
    compare_algo
    hypercube_core
    lsh_core
    visualization_core
    # Header-only libraries (INTERFACE targets)
    ivfflat_core
    ivfpq_core
    kmeans_core
    mnist_processing
    sift_processing
    metrics_core
    math_utils
)

# CTest + tests
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(test)
endif()

# (προαιρετικά) εγκατάσταση
install(TARGETS vss_app RUNTIME DESTINATION bin)
