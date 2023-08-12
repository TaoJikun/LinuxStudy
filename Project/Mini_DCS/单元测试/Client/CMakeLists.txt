cmake_minimum_required(VERSION 3.14)
project(client_unittest)
set(CMAKE_CXX_STANDARD 14)
add_executable(client_unittest client_unittest.cpp)
#下面这条语句中，链接了gmock、gtest以及pthread
#pthread是必要的，因为前两者会用到
TARGET_LINK_LIBRARIES(client_unittest gmock gtest pthread)
install(TARGETS client_unittest RUNTIME DESTINATION bin)
include(GoogleTest)
gtest_discover_tests(client_unittest)