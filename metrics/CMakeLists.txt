# Header-only library for distance metrics
add_library(metrics_core INTERFACE)
target_include_directories(metrics_core INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)