cmake_minimum_required(VERSION 3.14)
project(test_lru)
set(CMAKE_CXX_STANDARD 14)
add_executable(test_lru test_lru.cpp)
#下面这条语句中，链接了gmock、gtest以及pthread
#pthread是必要的，因为前两者会用到
TARGET_LINK_LIBRARIES(test_lru gmock gtest pthread)
install(TARGETS test_lru RUNTIME DESTINATION bin)
include(GoogleTest)
gtest_discover_tests(test_lru)