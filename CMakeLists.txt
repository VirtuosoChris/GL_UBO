cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(UBOTest)

find_package(OpenGL REQUIRED)

# add the executable
add_executable(std140Test test/main.cpp 
test/testshaders.h
)

set( VIRTUOSO_LIBS_DIR "usr/lib" CACHE PATH "Virtuosity repo BUILD home path" )

target_include_directories(std140Test PUBLIC "./test/depends/glfw/include")
target_include_directories(std140Test PUBLIC "./test/depends/")
target_include_directories(std140Test PUBLIC "./test/depends/glhpp/test/deps/")


target_link_libraries(std140Test OpenGL::GL)


target_compile_definitions(std140Test PUBLIC NOMINMAX)

add_subdirectory("./test/depends/glfw")

target_link_libraries(std140Test glfw)