cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(UBOTest)

find_package(OpenGL REQUIRED)


set (GLFWPP_SRC "./test/depends/glfwpp/src")


# add the executable
add_executable(std140Test test/main.cpp 
test/testshaders.h
test/depends/glad/src/glad.c
)

target_include_directories(std140Test PUBLIC "./test/depends/")
target_include_directories(std140Test PUBLIC "./test/depends/glhpp/test/deps/")
target_include_directories(std140Test PUBLIC "./test/depends/glad/include")


target_link_libraries(std140Test OpenGL::GL)

target_compile_definitions(std140Test PUBLIC NOMINMAX )

add_subdirectory("./test/depends/glfwpp")

target_include_directories(glfwpp PUBLIC ./test/depends/glfwpp/include/)

target_link_libraries(std140Test glfwpp)