
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

if(NOT MSVC AND CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()



if(NOT MSVC)
add_compile_options(-Wno-unused-function)
endif()

link_libraries(CURL::libcurl LibXml2::LibXml2 SQLite::SQLite3)

add_executable(parse_args_test parse_args.c)
add_executable(str_test str.c)
add_executable(db_test db.c)
add_executable(init_program_test init_program.c)
add_executable(transfer_mem_test transfer_mem.c)
add_executable(transfer_test transfer.c)
add_executable(db_unhappy_test db_unhappy.c)
add_executable(parse_test parse.c)
add_executable(rss_date_parse_test rss_date_parse.c)

add_test(NAME parse_args COMMAND parse_args_test)
add_test(NAME str COMMAND str_test)
add_test(NAME init_program COMMAND init_program_test)
add_test(NAME db COMMAND db_test)
add_test(NAME db_unhappy COMMAND db_unhappy_test)
add_test(NAME transfer_mem COMMAND transfer_mem_test)
add_test(NAME transfer COMMAND transfer_test)
add_test(NAME parse COMMAND parse_test)
add_test(NAME rss_date_parse COMMAND rss_date_parse_test)
