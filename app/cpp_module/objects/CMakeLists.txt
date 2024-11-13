add_library(Objects File.cpp File.h)

find_package(Drogon REQUIRED)

target_link_libraries(Objects PRIVATE Drogon::Drogon)