# Header-only library for IVFPQ
add_library(ivfpq_core INTERFACE)
target_include_directories(ivfpq_core INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)