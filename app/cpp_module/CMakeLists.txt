cmake_minimum_required(VERSION 3.5)

project(libForFileService, LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Drogon CONFIG REQUIRED)
find_package(jwt-cpp REQUIRED)
find_package(ZLIB REQUIRED)

# Add source directories
add_subdirectory(utils)
add_subdirectory(objects)
add_subdirectory(controllers)
add_subdirectory(filters)
add_subdirectory(plugins/JWT-drogon)

# Add executable
add_executable(StartPoint main.cpp)

# Link libraries
target_link_libraries(StartPoint 
    PRIVATE 
    FileController 
    AuthController
    JWTUtils
    JWTAuthFilter
    Drogon::Drogon
    jwt-cpp
    ZLIB::ZLIB
)

# Include directories
target_include_directories(StartPoint 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)