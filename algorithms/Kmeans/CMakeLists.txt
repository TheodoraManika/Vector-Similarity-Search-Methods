# Header-only library for Kmeans and Clustering
add_library(kmeans_core INTERFACE)
target_include_directories(kmeans_core INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)