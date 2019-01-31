cmake_minimum_required(VERSION 2.8.12)

project(tinyaes C ASM)

add_library(tiny-aes
        aes.c
        )

target_compile_definitions(tiny-aes PRIVATE
        -DAES128=1
        -DCBC=1
        -DECB=1
        -DCTR=1
        )

target_include_directories(tiny-aes PRIVATE tiny-AES-c/)
