#ifndef FEED_DB_H
#define FEED_DB_H
#include "fed.h"
#include "str.h"
#include <stdbool.h>

static bool initialize_db(fed *f) {
    char *errmsg = NULL;

    bool success = notnull(f) && notnull(f->conSqlite);

    if(success) {
        success = sqlite3_exec(f->conSqlite, "PRAGMA synchronous = NORMAL;", NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_exec(f->conSqlite, "PRAGMA journal_mode = \'wal\';", NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_exec(f->conSqlite, "BEGIN TRANSACTION;", NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_exec(f->conSqlite,
            "CREATE TABLE IF NOT EXISTS feeds ("
                "feed_id INTEGER NOT NULL CONSTRAINT pk_feeds PRIMARY KEY AUTOINCREMENT,"
                "url TEXT NOT NULL,"
                "etag TEXT NULL,"
                "last_modified TEXT NULL"
            ");",
            NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_exec(f->conSqlite,
            "CREATE UNIQUE INDEX IF NOT EXISTS uniq_feeds_url ON feeds (url);",
            NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_exec(f->conSqlite,
            "CREATE TABLE IF NOT EXISTS articles ("
                "article_id INTEGER NOT NULL CONSTRAINT pk_articles PRIMARY KEY AUTOINCREMENT,"
                "feed_id INTEGER NOT NULL,"
                "url TEXT NOT NULL,"
                "title TEXT NULL,"
                "updated TEXT NULL"
            ");",
            NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_exec(f->conSqlite,
            "CREATE UNIQUE INDEX IF NOT EXISTS uniq_articles_url ON articles (url);",
            NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_exec(f->conSqlite,
            "CREATE INDEX IF NOT EXISTS ix_articles_updated ON articles (updated);",
            NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_exec(f->conSqlite, "COMMIT;", NULL, NULL, &errmsg) == SQLITE_OK;
    }

    if(notnull(errmsg)) {
        // no need to check fprintf result, as we can do
        // nothing about it.
        (void)fprintf(stderr, "sqlite3 error: %s", errmsg);
        sqlite3_free(errmsg);
    }

    return success;
}


#endif
