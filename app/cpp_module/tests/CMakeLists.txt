enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    CompressionUtilsTest.cpp
    CacheManagerTest.cpp
    AsyncTaskManagerTest.cpp
    FileControllerTest.cpp
    CompressionStatsServiceTest.cpp
)

add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    ${PROJECT_NAME}_lib
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
