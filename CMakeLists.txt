cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME "http_server")

project(${PROJECT_NAME} LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    main.c)

add_executable(${PROJECT_NAME} ${SOURCES})