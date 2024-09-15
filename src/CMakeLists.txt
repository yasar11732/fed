
if(MSVC)
    add_definitions(/D_CONSOLE /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE /W4 /WX /wd4005 /wd4996 /nologo)
endif()

add_executable (fed main.c sqlite3.c)

target_link_libraries(fed PRIVATE CURL::libcurl LibXml2::LibXml2)
install(TARGETS fed DESTINATION bin)