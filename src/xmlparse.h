/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FED_PARSE_H
#define FED_PARSE_H
#include "fed.h"
#include "str.h"
#include "db.h"
#include "rss_date_parse.h"
#include <libxml/xmlreader.h>

static const int xmlReaderOptions = (
    XML_PARSE_RECOVER | // recover on errors
    XML_PARSE_NOCDATA | // merge cdata as text nodes
    XML_PARSE_NOBLANKS| // remove blank nodes
    XML_PARSE_NOENT   | // substitue entities
    XML_PARSE_NONET   | // forbid network access
    0
);

static const xmlChar *get_root_element(xmlTextReaderPtr xmlReader) {
  const xmlChar *root = NULL;

  while (xmlTextReaderRead(xmlReader) == 1) {
    if ((int)XML_READER_TYPE_ELEMENT == xmlTextReaderNodeType(xmlReader)) {
      root = xmlTextReaderConstName(xmlReader);
      break;
    }
  }

  return root;
}

static void process_rss(transfer_t *t, xmlTextReaderPtr r) {

    enum  {
        PARSE_STATE_NULL,
        PARSE_STATE_PARSING_ITEM,
        PARSE_STATE_PARSING_TITLE,
        PARSE_STATE_PARSING_LINK,
        PARSE_STATE_PARSING_PUBDATE
    } state = PARSE_STATE_NULL;

    xmlChar title[FED_MAXTITLE];
    xmlChar link[FED_MAXURL];
    xmlChar updated[FED_MAXTIMESTRING];
    char updated_sqlite_format[FED_MAXTIMESTRING];

    while(xmlTextReaderRead(r) == 1) {
        xmlReaderTypes node_type = xmlTextReaderNodeType(r);
        const char *tagName = (char*)xmlTextReaderConstName(r);
        switch(state) {
            case PARSE_STATE_NULL:
                if(node_type == XML_READER_TYPE_ELEMENT && streq(tagName, "item")) {
                    state = PARSE_STATE_PARSING_ITEM;
                    title[0] = '\0';
                    link[0] = '\0';
                    updated[0] = '\0';
                }
            break;
            case PARSE_STATE_PARSING_ITEM:
                if(node_type == XML_READER_TYPE_ELEMENT) {
                    if(streq(tagName, "title")) {
                        state = PARSE_STATE_PARSING_TITLE;
                    } else if (streq(tagName, "link")) {
                        state = PARSE_STATE_PARSING_LINK;
                    } else if (streq(tagName,"pubDate")) {
                        state = PARSE_STATE_PARSING_PUBDATE;
                    }
                } else if(node_type == XML_READER_TYPE_END_ELEMENT) {
                    if(streq(tagName,"item")) {
                        state = PARSE_STATE_NULL;
                        if(date_rss_to_sqlite((char*)updated, updated_sqlite_format)) {
                            insert_article(t, (char*)title, (char*)link, updated_sqlite_format);
                        } else {
                            fprintf(stderr, "%s Failed to parse rss date %s", t->url, updated);
                        }
                        
                    }
                }
            break;
            case PARSE_STATE_PARSING_TITLE:
                if(node_type == XML_READER_TYPE_TEXT) {
                    strncpy((char*)title, (char*)xmlTextReaderConstValue(r), FED_MAXTITLE-1);
                    title[FED_MAXTITLE-1] = '\0';
                } else if(node_type == XML_READER_TYPE_END_ELEMENT && streq((char*)tagName,"title")) {
                    state = PARSE_STATE_PARSING_ITEM;
                }
            break;
            case PARSE_STATE_PARSING_LINK:
                if(node_type == XML_READER_TYPE_TEXT) {
                    strncpy((char*)link, (char*)xmlTextReaderConstValue(r), FED_MAXURL-1);
                    link[FED_MAXURL-1] = '\0';
                } else if(node_type == XML_READER_TYPE_END_ELEMENT && streq((char*)tagName,"link")) {
                    state = PARSE_STATE_PARSING_ITEM;
                }
            break;
            case PARSE_STATE_PARSING_PUBDATE:
                if(node_type == XML_READER_TYPE_TEXT) {
                    strncpy((char*)updated, (char*)xmlTextReaderConstValue(r), FED_MAXTIMESTRING-1);
                    updated[FED_MAXTIMESTRING-1] = '\0';
                } else if(node_type == XML_READER_TYPE_END_ELEMENT && streq((char*)tagName,"pubDate")) {
                    state = PARSE_STATE_PARSING_ITEM;
                }
            break;
        }
    }
}

static void process_atom(transfer_t *t, xmlTextReaderPtr r) {
    enum {
        PARSE_STATE_NULL,
        PARSE_STATE_PARSING_ENTRY,
        PARSE_STATE_PARSING_TITLE,
        PARSE_STATE_PARSING_UPDATED
    } state = PARSE_STATE_NULL;

  xmlChar title[FED_MAXTITLE];
  xmlChar link[FED_MAXURL];
  xmlChar updated[FED_MAXTIMESTRING];

  while (xmlTextReaderRead(r) == 1) {
    const xmlChar *tagName = xmlTextReaderConstName(r);
    int node_type = xmlTextReaderNodeType(r);
    
    switch (state) {
    case PARSE_STATE_NULL:
      if (streq((char*)tagName, "entry")) {
        state = PARSE_STATE_PARSING_ENTRY;
        title[0] = '\0';
        link[0] = '\0';
        updated[0] = '\0';
      }
      break;
    case PARSE_STATE_PARSING_ENTRY:
      if (node_type == XML_READER_TYPE_ELEMENT) {
        if (streq((char*)tagName, "title")) {
          state = PARSE_STATE_PARSING_TITLE;
        } else if (streq((char*)tagName, "updated")) {
          state = PARSE_STATE_PARSING_UPDATED;
        } else if (streq((char*)tagName, "link")) {
          xmlChar *_link = xmlTextReaderGetAttribute(r, (xmlChar*)"href");
          strncpy((char*)link, (char*)_link, FED_MAXURL-1);
          link[FED_MAXURL-1] = '\0';
          xmlFree(_link);
        }
      } else if (node_type == XML_READER_TYPE_END_ELEMENT && streq((char*)tagName, "entry")) {
          state = PARSE_STATE_NULL;
          insert_article(t, (char*)title, (char*)link, (char*)updated);
      }
      break;
    case PARSE_STATE_PARSING_TITLE:
      if (node_type == XML_READER_TYPE_TEXT) {
        strncpy((char*)title, (char*)xmlTextReaderConstValue(r), FED_MAXTITLE-1);
        title[FED_MAXTITLE-1] = '\0';
      } else if (node_type == XML_READER_TYPE_END_ELEMENT  && streq((char*)tagName, "title")) {
          state = PARSE_STATE_PARSING_ENTRY;
      }
      break;
    case PARSE_STATE_PARSING_UPDATED:
      if (node_type == XML_READER_TYPE_TEXT) {
        strncpy((char*)updated, (char*)xmlTextReaderConstValue(r), FED_MAXTIMESTRING-1);
        updated[FED_MAXTIMESTRING-1] = '\0';
      } else if (node_type == XML_READER_TYPE_END_ELEMENT && streq((char*)tagName, "updated") ) {
          state = PARSE_STATE_PARSING_ENTRY;
      }
      break;
    }
  }
}

static void process_response(transfer_t *t) {
    const xmlChar *root;
    xmlTextReaderPtr reader = xmlReaderForMemory(t->data, (int)t->cbData, t->url, NULL, xmlReaderOptions);
    
    if(notnull(reader) && notnull(root = get_root_element(reader))) {
        if(streq((char*)root, "rss")) {
            process_rss(t, reader);
        } else if(streq((char*)root, "feed")) {
            process_atom(t, reader);
        } else {
            fprintf(stderr, "%s: Could not determine feed type.\n", t->url);
        }
    } else {
        fprintf(stderr, "%s: Could not parse xml response.\n", t->url);
    }

    if(notnull(reader)) {
        xmlFreeTextReader(reader);
    }
}

#endif
