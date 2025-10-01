add_library(visualization_core
  visualization.cpp
)
target_include_directories(visualization_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)
target_link_libraries(visualization_core PUBLIC hashtable)