cmake_minimum_required(VERSION 2.8.12)
project(TinyAesPackageTest C CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(example ../test.c)
add_executable(example_cpp ../test.cpp)

target_link_libraries(example ${CONAN_LIBS})
target_link_libraries(example_cpp ${CONAN_LIBS})