
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

if(NOT MSVC AND CMAKE_BUILD_TYPE MATCHES DEBUG)
add_compile_options(-fsanitize=address -fsanitize=undefined)
add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

add_executable(parse_args_test parse_args.c)
add_executable(str_test str.c)
add_executable(db_test db.c ../src/sqlite3.c)

add_executable(init_program_test init_program.c ../src/sqlite3.c)
target_link_libraries(init_program_test PRIVATE CURL::libcurl)

add_test(NAME parse_args COMMAND parse_args_test)
add_test(NAME str COMMAND str_test)
add_test(NAME init_program COMMAND init_program_test)
add_test(NAME db COMMAND db_test)
