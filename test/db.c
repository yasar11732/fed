#ifdef NDEBUG
#undef NDEBUG
#endif

#include "fed.h"
#include "db.h"
#include "sqlite3.h"
#include <assert.h>

int main() {
    sqlite3_stmt *res;
    fed f;
    init_fed(&f);
    bool success;
    
    success = sqlite3_open(":memory:",&f.conSqlite) == SQLITE_OK;
    assert(success);

    puts("Test initialize db creates tables.");
    success = initialize_db(&f);
    assert(success);

    success = sqlite3_prepare(f.conSqlite,
        "select name from sqlite_schema WHERE type=\"table\" and name=\"feeds\";",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);
    
    success = sqlite3_step(res) == SQLITE_ROW;
    assert(success);

    sqlite3_finalize(res);

    success = sqlite3_prepare(f.conSqlite,
        "select name from sqlite_schema WHERE type=\"table\" and name=\"articles\";",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);
    
    success = sqlite3_step(res) == SQLITE_ROW;
    assert(success);

    sqlite3_finalize(res);

    puts("Test initialize db creates indexes.");
    success = sqlite3_prepare(f.conSqlite,
        "select sql from sqlite_master WHERE type=\"index\" and name=\"uniq_feeds_url\";",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);
    
    success = sqlite3_step(res) == SQLITE_ROW;
    assert(success);

    const char *sql = sqlite3_column_text(res, 0);
    assert(strprefix(sql, "CREATE UNIQUE INDEX"));

    sqlite3_finalize(res);

    success = sqlite3_prepare(f.conSqlite,
        "select sql from sqlite_master WHERE type=\"index\" and name=\"uniq_articles_url\";",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);
    
    success = sqlite3_step(res) == SQLITE_ROW;
    assert(success);

    sql = sqlite3_column_text(res, 0);
    assert(strprefix(sql, "CREATE UNIQUE INDEX"));

    sqlite3_finalize(res);

    success = sqlite3_prepare(f.conSqlite,
        "select sql from sqlite_master WHERE type=\"index\" and name=\"ix_articles_updated\";",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);
    
    success = sqlite3_step(res) == SQLITE_ROW;
    assert(success);

    sql = sqlite3_column_text(res, 0);
    assert(strprefix(sql, "CREATE INDEX"));

    sqlite3_finalize(res);

    /*
    * insert some data for next tests
    */
   char *feed_mocks[3] = {
    "https://example1.com/feed.xml",
    "https://example2.com/feed.xml",
    "https://example3.com/feed.xml"
   };

   struct article_row {
    int feed_id;
    char *url;
   };

   struct article_row articles[6] = {
    {1,"https://example1.com/article1"},
    {1,"https://example1.com/article2"},
    {2,"https://example2.com/article1"},
    {2,"https://example2.com/article2"},
    {3,"https://example3.com/article1"},
    {3,"https://example3.com/article2"},
   };

    success = sqlite3_prepare(f.conSqlite,
        "insert into feeds (url) values(?);",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);

    for(size_t i = 0; i < (sizeof(feed_mocks)/sizeof(*feed_mocks)); i++) {
        assert(sqlite3_bind_text(res, 1, feed_mocks[i], -1, SQLITE_STATIC) == SQLITE_OK);
        assert(sqlite3_step(res) == SQLITE_DONE);
        sqlite3_reset(res);
    }

    sqlite3_finalize(res);

    success = sqlite3_prepare(f.conSqlite,
        "insert into articles (feed_id, url) values(?,?);",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);

    for(size_t i = 0; i < (sizeof(articles)/sizeof(*articles)); i++) {
        assert(sqlite3_bind_int(res, 1, articles[i].feed_id) == SQLITE_OK);
        assert(sqlite3_bind_text(res, 2, articles[i].url, -1, SQLITE_STATIC) == SQLITE_OK);
        assert(sqlite3_step(res) == SQLITE_DONE);
        sqlite3_reset(res);
    }

    sqlite3_finalize(res);

    printf("Test calling initialize again doesn't fail.");
    success = initialize_db(&f);
    assert(success);

    printf("Test calling initialize again keeps old data.");
    success = sqlite3_prepare(f.conSqlite,
        "select count(*) from feeds;",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);
    
    success = sqlite3_step(res) == SQLITE_ROW;
    assert(success);

    int rowcount = sqlite3_column_int(res, 0);
    assert(rowcount == 3);

    sqlite3_finalize(res);
        success = sqlite3_prepare(f.conSqlite,
        "select count(*) from articles;",
        -1, &res, NULL) == SQLITE_OK;
    assert(success);
    
    success = sqlite3_step(res) == SQLITE_ROW;
    assert(success);

    rowcount = sqlite3_column_int(res, 0);
    assert(rowcount == 6);
    sqlite3_finalize(res);

    sqlite3_close(f.conSqlite);
}