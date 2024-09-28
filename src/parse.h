#ifndef FED_PROCESS_H
#define FED_PROCESS_H
#include "fed.h"
#include "str.h"
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

static void process_rss(xmlTextReaderPtr reader) {
    (void)reader;
    printf("processing rss\n");
}

static void process_atom(xmlTextReaderPtr reader) {
    (void)reader;
    printf("processing atom\n");
}

static void process_response(transfer_t *t) {
    printf("Process response: %s\n", t->url);
    const xmlChar *root;
    xmlTextReaderPtr reader = xmlReaderForMemory(t->data, (int)t->cbData, t->url, NULL, xmlReaderOptions);
    
    if(notnull(reader) && notnull(root = get_root_element(reader))) {
        if(streq((char*)root, "rss")) {
            process_rss(reader);
        } else if(streq((char*)root, "feed")) {
            process_atom(reader);
        } else {
            fprintf(stderr, "%s: Could not determine feed type.\n", t->url);
        }
    } else {
        fprintf(stderr, "%s: Could not parse xml response.\n", t->url);
    }
}

#endif