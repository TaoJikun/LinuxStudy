cmake_minimum_required(VERSION 3.14)
project(test_thpl)
set(CMAKE_CXX_STANDARD 14)
add_executable(test_thpl test_thpl.cpp)
#下面这条语句中，链接了gmock、gtest以及pthread
#pthread是必要的，因为前两者会用到
TARGET_LINK_LIBRARIES(test_thpl gmock gtest pthread)
install(TARGETS test_thpl RUNTIME DESTINATION bin)
include(GoogleTest)
gtest_discover_tests(test_thpl)