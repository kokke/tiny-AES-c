cmake_minimum_required(VERSION 3.12)

project(tiny-aes C)

option(TINY_AES_ENABLE_AESNI "Enable runtime-dispatched AES-NI on x86" ON)

add_library(${PROJECT_NAME} "")
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/aes.c
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/aes.h
        ${CMAKE_CURRENT_LIST_DIR}/aes.hpp
)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" TINY_AES_PROCESSOR)
if(TINY_AES_ENABLE_AESNI AND TINY_AES_PROCESSOR MATCHES "^(amd64|x86_64|x64|i.86)$" AND
   CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang|MSVC)$")
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/aesni.c)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TINY_AES_AESNI=1)
    if(NOT MSVC)
        set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/aesni.c PROPERTIES COMPILE_OPTIONS "-maes")
    endif()
endif()

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
