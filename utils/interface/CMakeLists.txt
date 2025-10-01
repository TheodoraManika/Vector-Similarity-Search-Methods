add_library(interface_core
  Interface.cpp
)
target_include_directories(interface_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)
target_link_libraries(interface_core PUBLIC hashtable)