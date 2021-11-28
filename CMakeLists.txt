# 指定最小 CMake 版本要求
cmake_minimum_required(VERSION 3.9)
# 设置项目名称
project(sol2inter)

set(CMAKE_CXX_STANDARD 17)

#[[
添加可执行文件 target，类似于原来 Makefile 的：

    sol2inter: main.o
    main.o: main.cpp

CMake 会自动找到依赖的头文件，因此不需要特别指定，
当头文件修改的时候，会重新编译依赖它的目标文件。
#]]
add_executable(sol2inter main.cpp base.cpp token.cpp lexer.cpp parser.cpp labeler.cpp)

#[[
使用如下命令构建本项目：

    cmake -B build      # 生成构建目录
    cmake --build build # 执行构建
    ./build/sol2inter      # 运行 answer 程序
#]]