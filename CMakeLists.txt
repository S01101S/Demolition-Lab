cmake_minimum_required(VERSION 3.22)
project(Demolition_Lab)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(Demolition_Lab main.cpp)

target_link_libraries(Demolition_Lab PRIVATE sfml-graphics)



