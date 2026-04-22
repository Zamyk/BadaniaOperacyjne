cmake_minimum_required(VERSION 3.16)
project(BadaniaOperacyjne C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


add_executable(${PROJECT_NAME}
    main.c
    engine.c
    engine.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)