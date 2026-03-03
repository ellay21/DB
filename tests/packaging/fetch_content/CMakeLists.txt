cmake_minimum_required(VERSION 3.25)
project(revenant_fetch_content_test CXX)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(revenant
    SOURCE_DIR "${REVENANT_SOURCE_DIR}"
)
FetchContent_MakeAvailable(revenant)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE revenant::revenant)
