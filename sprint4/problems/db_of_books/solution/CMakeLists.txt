cmake_minimum_required(VERSION 3.11)

project(book_manager CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(book_manager
        src/main.cpp
        src/json_parse.h
        src/pg_database.h
)
target_include_directories(book_manager PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(book_manager PRIVATE CONAN_PKG::libpqxx
                                            CONAN_PKG::boost)
