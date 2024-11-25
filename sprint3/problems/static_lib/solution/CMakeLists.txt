cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CTest)
include(${CONAN_BUILD_DIRS_CATCH2}/Catch.cmake)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Добавляем библиотеку, указывая, что она статическая.
add_library(ModelLib STATIC
	src/game_session.h
	src/game_session.cpp
	src/loot_generator.h
	src/loot_generator.cpp
	src/model.h
	src/model.cpp
	src/tagged.h)

# Добавляем сторонние библиотеки. Указываем видимость PUBLIC, т. к.
# они должны быть ввидны и в библиотеке MyLib и в зависимостях.
target_include_directories(ModelLib PUBLIC ${ZLIB_INCLUDES})
target_link_libraries(ModelLib PUBLIC ${BOOST_LIB} ${ZLIB_LIB})

add_executable(game_server
	src/main.cpp
	src/command_line.h
	src/logging_handler.cpp
	src/logging_handler.h
	src/http_server.cpp
	src/http_server.h
	src/sdk.h
	src/players.h
	src/players.cpp
	src/boost_json.cpp
	src/json_loader.h
	src/json_loader.cpp
	src/request_handler.cpp
	src/request_handler.h
	src/ticker.h
	src/api_handler.h
	src/api_handler.cpp
)
# Добавляем зависимость целей от статической библиотеки.
target_link_libraries(game_server PRIVATE Threads::Threads
						CONAN_PKG::boost
						ModelLib)

add_executable(game_server_tests
	tests/model-tests.cpp
	tests/loot_generator_tests.cpp
)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2
						CONAN_PKG::boost
						Threads::Threads
						ModelLib)
catch_discover_tests(game_server_tests)
