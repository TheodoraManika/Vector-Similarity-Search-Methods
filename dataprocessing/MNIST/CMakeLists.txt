# Header-only library for MNIST data processing
add_library(mnist_processing INTERFACE)
target_include_directories(mnist_processing INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)