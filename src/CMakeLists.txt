# SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
#
# SPDX-License-Identifier: MIT

add_executable (fed main.c)
target_link_libraries(fed PRIVATE CURL::libcurl LibXml2::LibXml2 SQLite::SQLite3)
install(TARGETS fed DESTINATION bin)
