# Header-only library for IVFFlat
add_library(ivfflat_core INTERFACE)
target_include_directories(ivfflat_core INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)