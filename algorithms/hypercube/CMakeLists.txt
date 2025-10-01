add_library(hypercube_core
  Hypercube.cpp
)
target_include_directories(hypercube_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)
target_link_libraries(hypercube_core PUBLIC hashtable)