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
test/depends/glew-2.1.0/src/glew.c
)

target_include_directories(std140Test PUBLIC "./test/depends/glew-2.1.0/include")
target_include_directories(std140Test PUBLIC "./test/depends/glfw/include")
target_include_directories(std140Test PUBLIC "./test/depends/")
target_include_directories(std140Test PUBLIC "./test/depends/glhpp/test/deps/")


target_link_libraries(std140Test OpenGL::GL)

add_definitions(-DGLEW_STATIC)

target_compile_definitions(std140Test PUBLIC NOMINMAX )

add_subdirectory("./test/depends/glfw")


set(GLFW3_INCLUDE_DIR "./test/depends/glfw/include" CACHE PATH "The location of glfw.h")
set(GLFW3_LIB_FILE NONE CACHE FILEPATH "The location of glfw.lib")

add_library(glfwpp
	./test/depends/glfwpp/src/GLFW.cpp
	./test/depends/glfwpp/include/GLFW.hpp
	./test/depends/glfwpp/src/Monitor.cpp
	./test/depends/glfwpp/include/Monitor.hpp
	./test/depends/glfwpp/src/Window.cpp
	./test/depends/glfwpp/include/Window.hpp
	./test/depends/glfwpp/src/General.cpp
	./test/depends/glfwpp/include/General.hpp
	./test/depends/glfwpp/src/Joystick.cpp
	./test/depends/glfwpp/include/Joystick.hpp
	./test/depends/glfwpp/src/Cursor.cpp
	./test/depends/glfwpp/include/Cursor.hpp
)


target_include_directories(glfwpp PUBLIC ./test/depends/glfwpp/include/)

target_link_libraries(glfwpp glfw)

target_link_libraries(std140Test glfw)
target_link_libraries(std140Test glfwpp)