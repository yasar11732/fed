#ifndef FED_INIT_PROGRAM
#define FED_INIT_PROGRAM

#include "fed.h"
#include "str.h"

#include <stdbool.h>

static _Bool freadable(const char *path) {
    _Bool result;

    // opening the file is pretty portable
    // way to check if file exists
    // ? do we care about race condition ?
    FILE *f = fopen(path,"r");
    result = (f != NULL);
    if(result) {
        fclose(f);
    }
    return result;
}

static _Bool find_in_env(fed *f, const char *env, const char *format)
{
    _Bool success = false;
    const char *envstr = getenv(env);
    if(notnull(envstr)) {
        if(snprintf(f->pathUrls, MAX_PATH, format, envstr) <= MAX_PATH) {
            if(freadable(f->pathUrls)) {
                success = true;
            }
        }
    }
    return success;
}

static _Bool locate_urls_file(fed *f)
{    
    // if path explicitly defined on cmd
    // we don't do anything.
    _Bool success = !streq(f->pathUrls,"");
    
    if(!success) {
        success = find_in_env(f, "HOME", "%s/.fed/urls.txt");
    }

    if(!success) {
        success = find_in_env(f, "USERPROFILE", "%s\\.fed\\urls.txt");
    }

    if(!success) {
        success = find_in_env(f, "APPDATA", "%s\\.fed\\urls.txt");
    }

    if(!success) {
        char *varlib = "/var/lib/fed/urls.txt";
        if(freadable(varlib)) {
            strcpy(f->pathUrls, varlib);
            success = true;
        }

    }

    return success;
}

static _Bool open_urls_file(fed *f)
{
    _Bool success = locate_urls_file(f);
    
    if(success) {
        f->fileUrls = fopen(f->pathUrls, "r");
        success = (f->fileUrls != NULL);
    }

    return success;
}

static _Bool locate_db_file(fed *f) {
    _Bool success = !streq(f->pathDB,"");
    if(!success && !streq(f->pathUrls,"")) {
        strcpy(f->pathDB, f->pathUrls);
        stripfilename(f->pathDB);
        size_t len = strlen(f->pathDB);
        if(len + 7 < MAX_PATH) { // 1 for path separator, 6 for fed.db
            success = true;
            f->pathDB[len] = PATH_SEP;
            strcpy(&f->pathDB[len+1], "fed.db");
        }
    }
    return success;
}

static _Bool open_db_file(fed *f) {
    
    _Bool success = locate_db_file(f);

    if(success) {
        success = sqlite3_open(f->pathDB, &f->conSqlite) == SQLITE_OK;
    }

    return success;
}

static _Bool init_program(fed *f)
{
    _Bool success = open_urls_file(f);

    if(success) {
        success = open_db_file(f);
    }
    
    if(success) {

    }
    
    if(!success) {
        if(f->fileUrls != NULL) {
            fclose(f->fileUrls);
        }

        if(f->conSqlite != NULL) {
            sqlite3_close(f->conSqlite);
        }

    }
    return success;
}



#endif
