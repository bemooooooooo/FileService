add_library(JWTAuthFilter
    JWTAuthFilter.cpp
    JWTAuthFilter.hpp
)

target_link_libraries(JWTAuthFilter
    PRIVATE
    Drogon::Drogon
    JWTUtils
)

target_include_directories(JWTAuthFilter
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
