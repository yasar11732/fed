/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FED_IWRITER
#define FED_IWRITER
#include "fed.h"

typedef struct {
    void (*open)(fed *f);
    void (*write)(fed *f, const char *title, const char *url, const char *updated);
    void (*close)(fed *f);
} IWriter;

void txt_writer_open(fed *f) {
    (void)f;
}

void txt_writer_write(fed *f, const char *title, const char *url, const char *updated) {
    (void)f;
    printf("%s\t%s\t%s\r\n", title, url, updated);
}

void txt_writer_close(fed *f) {
    (void)f;
    fflush(stdout);
}

void xml_writer_open(fed *f) {
    (void)f;
}

void xml_writer_write(fed *f, const char *title, const char *url, const char *updated) {
    (void)f;
    (void)title;
    (void)url;
    (void)updated;
}

void xml_writer_close(fed *f) {
    (void)f;
}

void json_writer_open(fed *f) {
    (void)f;
}

void json_writer_write(fed *f, const char *title, const char *url, const char *updated) {
    (void)f;
    (void)title;
    (void)url;
    (void)updated;
}

void json_writer_close(fed *f) {
    (void)f;

}

enum Writers {
    TXT_WRITER,
    XML_WRITER,
    JSON_WRITER
};

static const IWriter Writers[] = {
    {txt_writer_open, txt_writer_write, txt_writer_close},
    {xml_writer_open, xml_writer_write, xml_writer_close},
    {json_writer_open, json_writer_write, json_writer_close},
};



#endif
