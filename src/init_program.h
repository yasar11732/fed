/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FED_INIT_PROGRAM
#define FED_INIT_PROGRAM

#include "fed.h"
#include "str.h"
#include "db.h"

#include <stdlib.h> // get env
#include <stdbool.h>
#include <curl/curl.h>

static bool freadable(const char *path) {
    
    assert(notnull(path));  // LCOV_EXCL_LINE

    FILE *f = fopen(path,"r");
    if(notnull(f)) {
        (void)fclose(f);
        return true;
    } else {
        return false;
    }
}

static bool find_in_env(fed *f, const char *env)
{
    assert(notnull(f));  // LCOV_EXCL_LINE
    assert(notnull(env));  // LCOV_EXCL_LINE

    const char *envstr = getenv(env);
    bool success = notnull(envstr);
    
    if(success) {
        success = path3cpy(f->pathUrls, envstr, ".fed","urls.txt");
    }

    if(success) {
        success = freadable(f->pathUrls);
    }

    return success;
}

static bool locate_urls_file(fed *f)
{
    assert(notnull(f));  // LCOV_EXCL_LINE

    // if path explicitly defined on cmd
    // we don't do anything.
    bool success = !streq(f->pathUrls,"");
    
#ifdef ON_WINDOWS
    if(!success) {
        success = find_in_env(f, "USERPROFILE");
    }

    if(!success) {
        success = find_in_env(f, "APPDATA");
    }
#else
    if(!success) {
        success = find_in_env(f, "HOME");
    }


    if(!success) {
        const char *varlib = "/var/lib/fed/urls.txt";
        if(freadable(varlib)) {
            success = path1cpy(f->pathUrls, varlib);
        }

    }
#endif
    return success;
}

static bool open_urls_file(fed *f)
{
    assert(notnull(f));  // LCOV_EXCL_LINE

    bool success = locate_urls_file(f);
    
    if(success) {
        f->fileUrls = fopen(f->pathUrls, "r");
        success = (f->fileUrls != NULL);
    }

    if(!success) {
        assert(f->fileUrls == NULL);  // LCOV_EXCL_LINE
    }

    return success;
}

static bool locate_db_file(fed *f) {

    assert(notnull(f));  // LCOV_EXCL_LINE
    
    bool success = !streq(f->pathDB,"");
    if(!success && !streq(f->pathUrls,"")) {
        
        success = path1cpy(f->pathDB, f->pathUrls);
        assert(success);  // LCOV_EXCL_LINE
        stripfilename(f->pathDB);
        success = path1cat(f->pathDB, "fed.db");
    }
    return success;
}

static bool open_db_file(fed *f) {
    
    assert(notnull(f));  // LCOV_EXCL_LINE
    
    bool success = locate_db_file(f);
    
    if(success) {
        success = sqlite3_open(f->pathDB, &f->conSqlite) == SQLITE_OK;
    }

    return success;
}



static bool init_program(fed *f)
{
    assert(notnull(f));  // LCOV_EXCL_LINE

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4127)
#endif
    if(CURLMSG_LAST != 2) {
        fprintf(stderr, "Incompatible curl version detected. If you experience problems, try downgrading curl or upgrading this program.");
    }
#ifdef _MSC_VER
    #pragma warning( pop )
#endif
    
    bool success = open_urls_file(f);

    if(success) {
        success = open_db_file(f);
    }
    
    if(success) {
        success = initialize_db(f);
    }

    if(success) {
        success = curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK;
    }

    if(success) {
        success = notnull(f->mh = curl_multi_init());
    }

    if(success) {
        (void)curl_multi_setopt(f->mh, CURLMOPT_MAXCONNECTS, FED_MAXPARALLEL);
    }

    return success;
}

static bool cleanup_program(fed *f) {
    
    bool success = true;

    (void)curl_global_cleanup();

    if(notnull(f->fileUrls)) {
        success = (fclose(f->fileUrls) != EOF);
    }

    if(notnull(f->conSqlite)) {
        success = (sqlite3_close(f->conSqlite) == SQLITE_OK) && success;
    }

    if(notnull(f->mh)) {
        success = (curl_multi_cleanup(f->mh) == CURLM_OK) && success;
    }

    return success;
}

#endif
