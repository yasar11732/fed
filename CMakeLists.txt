cmake_minimum_required (VERSION 3.15)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

project ("fed" LANGUAGES C)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

add_subdirectory(src)

option(BUILD_TESTS "Build Test Programs" OFF)

if(BUILD_TESTS)
	include(CTest)
	enable_testing()
	add_subdirectory(test)
endif()



