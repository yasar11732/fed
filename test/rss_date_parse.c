#include "rss_date_parse.h"

int main() {
    char sqlitedate[32];
    date_rss_to_sqlite("Thu, 12 Oct 2023 00:00:00 +0000", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T00:00:00+00:00"));
}