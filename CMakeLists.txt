cmake_minimum_required(VERSION 3.12.1)

project(gamepad VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_executable(${PROJECT_NAME}
  src/main.cpp
)
