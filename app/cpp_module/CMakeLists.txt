cmake_minimum_required(VERSION 3.15)

project(libForFileService, LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Drogon REQUIRED)

add_executable(StartPoint main.cpp)

target_link_libraries(StartPoint PRIVATE libcpp)
target_link_libraries(StartPoint PRIVATE Drogon::Drogon)
