cmake_minimum_required(VERSION 3.23)

project(SuperWSServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_library(UV_LIB libuv.so)
find_package(nlohmann_json 3.12.0 REQUIRED)

include_directories($ENV{HOME}/.local/include)
link_directories(/usr/local/lib)

add_executable(SuperWSServer main.cpp)
target_link_libraries(SuperWSServer
	PRIVATE
	$ENV{HOME}/.local/lib/uSockets.a
	OpenSSL::SSL
	ZLIB::ZLIB ${UV_LIB}
	nlohmann_json::nlohmann_json
)

