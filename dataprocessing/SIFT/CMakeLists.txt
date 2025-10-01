# Header-only library for SIFT data processing
add_library(sift_processing INTERFACE)
target_include_directories(sift_processing INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)