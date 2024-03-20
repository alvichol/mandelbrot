cmake_minimum_required(VERSION 3.28)
project(mandelbrot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(mandelbrot main.cpp src/main_window.cpp src/main_window.h src/worker.cpp src/worker.h)

target_link_libraries(mandelbrot PRIVATE Qt6::Core Qt6::Widgets)
