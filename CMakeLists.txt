cmake_minimum_required(VERSION 3.10)

project(solidity-vulnerability-checker)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall --std=c++17 -O3 -fopenmp -lthrift")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/app)

set(THIRD_PARTY_PATH "${CMAKE_SOURCE_DIR}/third_party")
set(THIRD_PARTY_INC_PATH "${THIRD_PARTY_PATH}/include")

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "Building on Mac.")
    set(THIRD_PARTY_LIB_PATH "${THIRD_PARTY_PATH}/lib/mac")
    set(THIRD_PARTY_BIN_PATH "${THIRD_PARTY_PATH}/bin/mac")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "Building on Linux.")
    set(THIRD_PARTY_LIB_PATH "${THIRD_PARTY_PATH}/lib/linux")
    set(THIRD_PARTY_BIN_PATH "${THIRD_PARTY_PATH}/bin/linux")
endif()

set(BASE_PATH "${CMAKE_SOURCE_DIR}")

include_directories("${BASE_PATH}")
include_directories(${THIRD_PARTY_INC_PATH})
include_directories("/usr/local/include")
link_directories(${THIRD_PARTY_LIB_PATH})
link_directories("/usr/local/lib")

include_directories(utils)
include_directories(processor)
include_directories(lexer)
include_directories(ast)
include_directories(gen_cpp)

add_subdirectory(utils)
add_subdirectory(processor)
add_subdirectory(lexer)
add_subdirectory(ast)
add_subdirectory(gen_cpp)

set(SOURCES main.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(build-dataset build_dataset.cpp)