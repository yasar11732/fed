#ifndef FED_INIT_PROGRAM
#define FED_INIT_PROGRAM

#include "fed.h"
#include "str.h"
#include "db.h"

#include <stdbool.h>
#include <curl/curl.h>

static bool freadable(const char *path) {
    
    bool result = notnull(path);
    FILE *f = NULL;

    // opening the file is pretty portable
    // way to check if file exists
    // ? do we care about race condition ?
    if(result) {
        f = fopen(path,"r");
        result = (f != NULL);
    }

    if(result) {
        fclose(f);
    }

    return result;
}

static bool find_in_env(fed *f, const char *env)
{
    bool success = notnull(f) && notnull(env);
    const char *envstr = NULL;

    if(success) {
        envstr = getenv(env);
        success = notnull(envstr);
    }
    
    if(success) {
        if(path3cpy(f->pathUrls, envstr, ".fed","urls.txt")) {
            if(freadable(f->pathUrls)) {
                success = true;
            }
        }
    }

    return success;
}

static bool locate_urls_file(fed *f)
{    
    // if path explicitly defined on cmd
    // we don't do anything.
    if(f == NULL)
        return false;
    
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
        char *varlib = "/var/lib/fed/urls.txt";
        if(freadable(varlib)) {
            path1cpy(f->pathUrls, varlib);
            success = true;
        }

    }
#endif
    return success;
}

static bool open_urls_file(fed *f)
{
    bool success = notnull(f);

    if(success) {
        success = locate_urls_file(f);
    }
    
    if(success) {
        f->fileUrls = fopen(f->pathUrls, "r");
        success = (f->fileUrls != NULL);
    }

    if(!success) {
        /*
        * At this point fileUrls is already 0
        * but NULL is implementation-defined.
        * we shouldn't assume NULL == 0
        */
        f->fileUrls = NULL;
    }

    return success;
}

static bool locate_db_file(fed *f) {
    
    if(f == NULL)
        return false;
    
    bool success = !streq(f->pathDB,"");
    if(!success && !streq(f->pathUrls,"")) {
        
        // Unlikely to fail, but still performing the test for robustness.
        success = path1cpy(f->pathDB, f->pathUrls);
        if(success) {
            stripfilename(f->pathDB);
            success = path1cat(f->pathDB, "fed.db");
        }
    }
    return success;
}

static bool open_db_file(fed *f) {
    
    bool success = notnull(f);
    
    if(success) {
        success = locate_db_file(f);
    }

    if(success) {
        success = sqlite3_open(f->pathDB, &f->conSqlite) == SQLITE_OK;
    }

    return success;
}



static bool init_program(fed *f)
{
    bool success = notnull(f);
    
    if(success) {
        success = open_urls_file(f);
    }

    if(success) {
        success = open_db_file(f);
    }
    
    if(success) {
        success = initialize_db(f);
    }

    if(success) {
        success = curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK;
    }

    return success;
}

static bool cleanup_program(fed *f) {
    
    bool success = true;

    (void)curl_global_cleanup();

    if(f->fileUrls != NULL) {
        success = (fclose(f->fileUrls) != EOF);
    }

    if(f->conSqlite != NULL) {
        success = (success && (sqlite3_close(f->conSqlite) == SQLITE_OK));
    }

    return success;
}

#endif
