cmake_minimum_required(VERSION 3.10)

# set(CMAKE_C_COMPILER "clang")
project(Practice C)

file(
	GLOB
	SOURCES
	"src/main.c"
)

add_executable(dummy ${SOURCES})

target_include_directories(
	dummy
	PRIVATE
	"${CMAKE_SOURCE_DIR}/../SDL/include" "${CMAKE_SOURCE_DIR}/../glad/include"
)

