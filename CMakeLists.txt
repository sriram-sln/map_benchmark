cmake_minimum_required(VERSION 3.16)

project(map_benchmark)

find_package(
    Boost REQUIRED COMPONENTS container
)

# Abseil requires C++14
set(CMAKE_CXX_STANDARD 20)

# Process Abseil's CMake build system
add_subdirectory(thirdparty/abseil-cpp)

add_executable(main main.cpp)

target_link_libraries(main 
    absl::flat_hash_map Boost::container)