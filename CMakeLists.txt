# SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

project ("fed" LANGUAGES C)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(SQLite3 REQUIRED)

if(MSVC)
    add_definitions(/D_CONSOLE /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE)
    add_compile_options(/W4 /WX /wd4005 /wd4996 /nologo)
else()
    add_compile_options(-pedantic -pedantic-errors -Wall -Wextra -Werror)
endif()

add_subdirectory(src)

option(BUILD_TESTS "Build Test Programs" OFF)
option(ENABLE_COVERAGE "Build with --coverage" OFF)


if(MSVC AND ENABLE_COVERAGE)
    message(FATAL_ERROR "Cannot ENABLE_COVERAGE on MSVC compiler.")
endif()

if(ENABLE_COVERAGE)
	add_compile_options(--coverage -g -O0)
	add_link_options(--coverage)
endif()

if(BUILD_TESTS)
	include(CTest)
	enable_testing()
	add_subdirectory(test)
endif()


