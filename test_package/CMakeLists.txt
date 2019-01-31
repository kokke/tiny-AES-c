cmake_minimum_required(VERSION 2.8.12)
project(TinyAesPackageTest C)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(example ../test.c)
target_link_libraries(example ${CONAN_LIBS})
