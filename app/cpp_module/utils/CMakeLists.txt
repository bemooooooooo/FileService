add_library(JWTUtils
    JWTUtils.cpp
    JWTUtils.hpp
)

target_link_libraries(JWTUtils
    PRIVATE
    jwt-cpp
)

target_include_directories(JWTUtils
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
