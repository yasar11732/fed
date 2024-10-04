#ifndef FED_DB_H
#define FED_DB_H
#include "fed.h"
#include "str.h"
#include <stdbool.h>
#include <assert.h>

static bool initialize_db(fed *f) {
    char *errmsg = NULL;

    assert(notnull(f));  // LCOV_EXCL_LINE
    assert(notnull(f->conSqlite));  // LCOV_EXCL_LINE

    bool success = sqlite3_exec(f->conSqlite, "PRAGMA synchronous = NORMAL;", NULL, NULL, &errmsg) == SQLITE_OK;

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

static bool get_feed_details(transfer_t *t) {

    /*
    * Insert this url to db if it is not added previously
    */
    sqlite3_stmt *stmt;
    bool success = sqlite3_prepare(t->fed->conSqlite,
        "INSERT OR IGNORE INTO feeds (url) VALUES (?)",
        -1, &stmt, NULL) == SQLITE_OK;
    
    if(success) {
        success = sqlite3_bind_text(stmt, 1, t->url, -1, SQLITE_STATIC) == SQLITE_OK;;
    }
    
    if(success) {
        success = sqlite3_step(stmt) == SQLITE_DONE;
    }

    if(notnull(stmt)) {
        sqlite3_finalize(stmt);
    }

    stmt = NULL;

    if(success) {
        success = sqlite3_prepare(
            t->fed->conSqlite,
                "SELECT feed_id,etag,last_modified FROM feeds WHERE url=?",
                -1, &stmt, NULL) == SQLITE_OK;
    }
    
    if(success) {
        success = sqlite3_bind_text(stmt, 1, t->url, -1, SQLITE_STATIC) == SQLITE_OK;
    }
    
    if(success) {
        success =  sqlite3_step(stmt) == SQLITE_ROW;
    }

    if(success) {
        t->feed_id = sqlite3_column_int(stmt, 0);
        success = t->feed_id != 0;
    }
   
    if(success) {
        const char *val = (const char *)sqlite3_column_text(stmt,1);
        if(notnull(val)) {
            strcpy(t->etag, val);
        }
    }

    if(success) {
        const char *val = (const char *)sqlite3_column_text(stmt,2);
        if(notnull(val)) {
            strcpy(t->lastmodified, val);
        }
    }

    if(stmt != NULL) {
        sqlite3_finalize(stmt);
    }

    return success;
}

static bool save_feed_details(transfer_t *t) {
    sqlite3_stmt *stmt;
    bool success = sqlite3_prepare(t->fed->conSqlite,
        "UPDATE feeds SET etag=?, last_modified=? WHERE feed_id=?",
        -1, &stmt, NULL) == SQLITE_OK;

    if(success) {
        success = sqlite3_bind_text(stmt, 1, t->etag, -1, SQLITE_STATIC) == SQLITE_OK;;
    }

    if(success) {
        success = sqlite3_bind_text(stmt, 2, t->lastmodified, -1, SQLITE_STATIC) == SQLITE_OK;;
    }

    if(success) {
        success = sqlite3_bind_int(stmt, 3, t->feed_id) == SQLITE_OK;;
    }
    
    if(success) {
        success = sqlite3_step(stmt) == SQLITE_DONE;
    }

    if(notnull(stmt)) {
        sqlite3_finalize(stmt);
    }

    if(!success) {
        fprintf(stderr,"%s", sqlite3_errmsg(t->fed->conSqlite));
    }
    return success;
    
}

static bool insert_article(transfer_t *t, char *title, char *link, char *updated){
    sqlite3_stmt *stmt;
    bool success = sqlite3_prepare(t->fed->conSqlite,
        "INSERT OR REPLACE INTO articles(feed_id, url, title, updated) VALUES (?,?,?,?)",
        -1, &stmt, NULL) == SQLITE_OK;

    if(success) {
        success = sqlite3_bind_int(stmt, 1, t->feed_id) == SQLITE_OK; 
    }

    if(success) {
        success = sqlite3_bind_text(stmt, 2, link, -1, SQLITE_STATIC) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_bind_text(stmt, 3, title, -1, SQLITE_STATIC) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_bind_text(stmt, 4, updated, -1, SQLITE_STATIC) == SQLITE_OK;
    }

    if(success) {
        success = sqlite3_step(stmt) == SQLITE_DONE;
    }

    if(notnull(stmt)) {
        success = (sqlite3_finalize(stmt) == SQLITE_OK) && success;
    }

    return success;

}


#endif
