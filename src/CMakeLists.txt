

add_executable (fed main.c sqlite3.c)
target_link_libraries(fed PRIVATE CURL::libcurl LibXml2::LibXml2)
install(TARGETS fed DESTINATION bin)
