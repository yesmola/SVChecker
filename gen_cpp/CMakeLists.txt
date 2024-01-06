cmake_minimum_required(VERSION 3.10)

project(rpc-server)

add_executable(${PROJECT_NAME} rpc_server.cpp MyService.cpp test_constants.cpp test_types.cpp)
target_link_libraries(${PROJECT_NAME} thrift Utils Processor Lexer)
