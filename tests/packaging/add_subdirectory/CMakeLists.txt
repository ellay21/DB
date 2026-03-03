cmake_minimum_required(VERSION 3.25)
project(revenant_add_subdirectory_test CXX)
set(CMAKE_CXX_STANDARD 20)

set(REVENANT_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${REVENANT_SOURCE_DIR}" revenant)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE revenant::revenant)
