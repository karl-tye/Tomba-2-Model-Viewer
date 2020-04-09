cmake_minimum_required(VERSION 3.14)
project(TombaOBJEditor C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

set(CMAKE_C_STANDARD 99)

add_executable(TombaOBJEditor main.c smst.h smst.c tombaex.c tombaex.h)

target_link_libraries(TombaOBJEditor -lopengl32 -lfreeglut -lglu32)