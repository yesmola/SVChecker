cmake_minimum_required(VERSION 3.10)

project(Lexer)

file(GLOB_RECURSE include "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE src "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} ${src} ${include})
target_link_libraries(${PROJECT_NAME} Utils)
