cmake_minimum_required (VERSION 2.8)

project (demo)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory (src SRC_LIST)

include_directories (include)

add_executable (main.out ${SRC_LIST})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(main.out PUBLIC Threads::Threads)
