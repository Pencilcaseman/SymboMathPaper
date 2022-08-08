cmake_minimum_required(VERSION 3.23)
project(SymboMath)

set(CMAKE_CXX_STANDARD 17)

add_executable(SymboMath main.cpp)

set(LIBRAPID_USE_MULTIPREC ON)
add_subdirectory(librapid)
target_link_libraries(SymboMath PUBLIC librapid)
