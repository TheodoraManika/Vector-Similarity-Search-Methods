add_library(compare_algo
  compareAlgo.cpp
)
target_include_directories(compare_algo PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)
target_link_libraries(compare_algo PUBLIC hashtable)