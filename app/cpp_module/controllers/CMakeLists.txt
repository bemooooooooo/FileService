add_library(FileController FileController.cpp FileController.h)

find_package(Drogon REQUIRED)

target_link_libraries(libcpp PRIVATE Drogon::Drogon)