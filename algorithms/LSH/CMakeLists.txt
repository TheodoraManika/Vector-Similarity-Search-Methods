add_library(lsh_core
  LSH.cpp
)
target_include_directories(lsh_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)
target_link_libraries(lsh_core PUBLIC hashtable)