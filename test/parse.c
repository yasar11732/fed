#ifdef NDEBUG
#undef NDEBUG
#endif
#define FED_PARSE_H
#include "fed.h"
#include "str.h"
#include "db.h"
#include <stdio.h>
#include <assert.h>

#include "xml1.h"
#include "xml2.h"

transfer_t local_t;
fed local_fed;


int insert_article_calls = 0;
char *expected_titles[5];
char *expected_links[5];
char *expected_updated[5];

void insert_article_mock(fed *f, char *title, char *link, char *updated) {
    (void)f;

    assert(streq(title, expected_titles[insert_article_calls]));
    assert(streq(link, expected_links[insert_article_calls]));
    assert(streq(updated, expected_updated[insert_article_calls]));

    insert_article_calls++;
}

void begin_test(char*msg) {
    init_fed(&local_fed);
    init_transfer(&local_t);
    memcpy(expected_titles, &(char*[5]){NULL, NULL, NULL, NULL, NULL},sizeof(char*[5]));
    memcpy(expected_links, &(char*[5]){NULL, NULL, NULL, NULL, NULL},sizeof(char*[5]));
    memcpy(expected_updated, &(char*[5]){NULL, NULL, NULL, NULL, NULL},sizeof(char*[5]));
    insert_article_calls = 0;
    puts(msg);
}

#define insert_article insert_article_mock

#undef FED_PARSE_H
#include "xmlparse.h"


int main() {
    /*
    * Basic tests 
    */

    begin_test("Empty feed");
    strcpy(local_t.url, "https://www.example.com/feed.xml");
    strcpy(local_t.data, "<feed xmlns=\"http://www.w3.org/2005/Atom\"></feed>");
    local_t.cbData = strlen(local_t.data);
    process_response(&local_fed,&local_t);
    assert(insert_article_calls == 0);

    begin_test("Empty rss");
    strcpy(local_t.data, "<rss xmlns:atom=\"http://www.w3.org/2005/Atom\" xmlns:content=\"http://purl.org/rss/1.0/modules/content/\" version=\"2.0\">");
    local_t.cbData = strlen(local_t.data);
    process_response(&local_fed,&local_t);
    assert(insert_article_calls == 0);

    begin_test("Empty rss with comment.");
    strcpy(local_t.data, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!-- comment --><rss></rss");
    local_t.cbData = strlen(local_t.data);
    process_response(&local_fed, &local_t);
    assert(insert_article_calls == 0);

    begin_test("Invalid feed");
    strcpy(local_t.data, "<unknownroot></unknownroot>");
    local_t.cbData = strlen(local_t.data);
    process_response(&local_fed, &local_t);
    assert(insert_article_calls == 0);

    begin_test("empty feed.");
    strcpy(local_t.data, "");
    local_t.cbData = strlen(local_t.data);
    process_response(&local_fed, &local_t);
    assert(insert_article_calls == 0);

    begin_test("huge feed.");
    // test out of memory
    local_t.cbData = (size_t)-1;
    process_response(&local_fed, &local_t);
    assert(insert_article_calls == 0);

    begin_test("xml1");
    memcpy(local_t.data, xml1, sizeof(xml1));
    local_t.cbData = sizeof(xml1);
    expected_titles[0] = "A journey through KiUserExceptionDispatcher";
    expected_links[0] = "https://momo5502.com/posts/2024-09-07-a-journey-through-kiuserexceptiondispatcher/";
    expected_updated[0] = "Sat, 07 Sep 2024 00:00:00 +0000";
    process_response(&local_fed, &local_t);
    assert(insert_article_calls == 1);

    begin_test("xml2");
    memcpy(local_t.data, xml2, sizeof(xml2));
    local_t.cbData = sizeof(xml2);
    expected_titles[0] = "CrowdStrike abuse campaign";
    expected_links[0] = "https://amr-git-dot.github.io/malware%20analysis/CrowdStrike_Abuse_Campaign/";
    expected_updated[0] = "2024-07-21T00:00:00+00:00";
    process_response(&local_fed, &local_t);
    assert(insert_article_calls == 1);

}