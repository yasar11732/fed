
add_executable (fed main.c sqlite3.c)

if(MSVC)
    add_definitions(/D_CONSOLE /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE)
    add_compile_options(/W4 /WX /wd4005 /wd4996 /nologo)
else()
    add_compile_options(-pedantic-errors -Wall -Wextra -Werror)
endif()

target_link_libraries(fed PRIVATE CURL::libcurl LibXml2::LibXml2)
install(TARGETS fed DESTINATION bin)
