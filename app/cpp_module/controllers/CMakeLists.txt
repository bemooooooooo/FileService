add_library(FileController FileController.cpp FileController.h)

find_package(Drogon REQUIRED)

target_include_directories(FileController PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(FileController PRIVATE Objects Drogon::Drogon)
