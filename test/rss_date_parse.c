/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "rss_date_parse.h"

int main() {
    char sqlitedate[32];

    // with positive timezone
    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 +0500", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34+05:00"));

    // with negative timezone
    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 -0400", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-04:00"));

    // with 2 digit year
    date_rss_to_sqlite("Thu, 12 Oct 75 09:12:34 -0400", sqlitedate);
    assert(streq(sqlitedate,"1975-10-12T09:12:34-04:00"));

    // with 1 digit day
    date_rss_to_sqlite("Thu, 5 Oct 75 09:12:34 -0400", sqlitedate);
    assert(streq(sqlitedate,"1975-10-05T09:12:34-04:00"));

    // without seconds
    date_rss_to_sqlite("Thu, 12 Oct 75 09:12 -0400", sqlitedate);
    assert(streq(sqlitedate,"1975-10-12T09:12:00-04:00"));

    // without day of the week
    date_rss_to_sqlite("12 Oct 75 09:12 -0400", sqlitedate);
    assert(streq(sqlitedate,"1975-10-12T09:12:00-04:00"));

    // technically, rfc822 allows tabs between tokens
    // probably never used in reality.
    date_rss_to_sqlite("Thu,\t12\tOct\t2023\t09:12:34\t+0500", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34+05:00"));

    /*
    * TIMEZONE TESTS
    */
    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 GMT", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34+00:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 UT", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34+00:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 EST", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-05:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 EDT", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-04:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 CST", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-06:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 CDT", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-05:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 MST", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-07:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 MDT", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-06:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 PST", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-08:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 PDT", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-07:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 Z", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34+00:00"));
    
    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 A", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-01:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 M", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34-12:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 N", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34+01:00"));

    date_rss_to_sqlite("Thu, 12 Oct 2023 09:12:34 Y", sqlitedate);
    assert(streq(sqlitedate,"2023-10-12T09:12:34+12:00"));

    /*
    * FAILURE CONDITIONS
    */

   // test for invalid character at any position
   char rssdate[] = "Thu, 12 Oct 2023 09:12:34 +0500";
   for(size_t i = 0; i < sizeof(rssdate)-1; i++) {
        char saved = rssdate[i];
        rssdate[i] = '$';
        printf("Parsing %s\r\n",rssdate);
        assert(!date_rss_to_sqlite(rssdate, sqlitedate));
        rssdate[i] = saved;
   }

   char rssdate2[] = "Thu, 12 Oct 2023 09:12:34 GMT";
   for(size_t i = 0; i < sizeof(rssdate2)-1; i++) {
        char saved = rssdate2[i];
        rssdate2[i] = '$';
        printf("Parsing %s\r\n",rssdate2);
        assert(!date_rss_to_sqlite(rssdate2, sqlitedate));
        rssdate2[i] = saved;
   }

   char rssdate3[] = "12 Oct 2023 09:12:34 Z";
   for(size_t i = 0; i < sizeof(rssdate3)-1; i++) {
        char saved = rssdate3[i];
        rssdate3[i] = '$';
        printf("Parsing %s\r\n",rssdate3);
        assert(!date_rss_to_sqlite(rssdate3, sqlitedate));
        rssdate3[i] = saved;
   }

   char rssdate4[] = "12 Oct 72 09:12 M";
   for(size_t i = 0; i < sizeof(rssdate4)-1; i++) {
        char saved = rssdate4[i];
        rssdate4[i] = '$';
        printf("Parsing %s\r\n",rssdate4);
        assert(!date_rss_to_sqlite(rssdate4, sqlitedate));
        rssdate4[i] = saved;
   }

   // test for short strings
   char rssdate5[] = "Thu, 12 Oct 2023 09:12:34 +0500";
   for(size_t i = sizeof(rssdate5)-2; i > 0; i--) {
        rssdate5[i] = '\0';
        assert(!date_rss_to_sqlite(rssdate5, sqlitedate));
   }


   // 2 consecutive spaces are invalid
   assert(!date_rss_to_sqlite("Thu,  12 Oct 2023 09:12:34 +0500", sqlitedate));
}