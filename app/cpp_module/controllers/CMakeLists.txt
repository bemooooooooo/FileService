add_library(FileController
    FileController.cpp
    FileController.h
)

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(FileController
    PRIVATE
    Drogon::Drogon
    JWTUtils
    stduuid
)

target_include_directories(FileController
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
