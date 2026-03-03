cmake_minimum_required(VERSION 3.25)
project(revenant_find_package_test CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(revenant REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE revenant::revenant)
