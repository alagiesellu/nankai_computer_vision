cmake_minimum_required(VERSION 3.20)
project(nankai_computer_vision)

set(CMAKE_CXX_STANDARD 20)

add_executable(nankai_computer_vision main.cpp libs/stb_image.h libs/stb_image_write.h)
