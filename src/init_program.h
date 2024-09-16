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

static _Bool find_in_env(fed *f, const char *env)
{
    _Bool success = false;
    const char *envstr = getenv(env);
    if(notnull(envstr)) {
#ifdef ON_WINDOWS
        if(snprintf(f->pathUrls, FED_MAXPATH, "%s\\.fed\\urls.txt", envstr) < FED_MAXPATH) {
#else
        if(snprintf(f->pathUrls, FED_MAXPATH, "%s/.fed/urls.txt", envstr) < FED_MAXPATH) {
#endif
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
            strcpy(f->pathUrls, varlib);
            success = true;
        }

    }
#endif
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
        puts(f->pathUrls);
        strcpy(f->pathDB, f->pathUrls);
        stripfilename(f->pathDB);
        puts(f->pathDB);
        size_t len = strlen(f->pathDB);
        if(len + 7 < FED_MAXPATH) { // 1 for path separator, 6 for fed.db
            success = true;
            f->pathDB[len] = PATH_SEP;
            strcpy(&f->pathDB[len+1], "fed.db");
            puts(f->pathDB);
        }
    }
    return success;
}

static _Bool open_db_file(fed *f) {
    
    _Bool success = locate_db_file(f);

    if(success) {
        puts(f->pathDB);
        success = sqlite3_open(f->pathDB, &f->conSqlite) == SQLITE_OK;
    } else {
        puts("locate db file failed.");
    }

    if(!success) {
        puts("sqlite3_open failed.");
    }

    return success;
}

static _Bool init_program(fed *f)
{
    _Bool success = open_urls_file(f);

    if(success) {
        success = open_db_file(f);
    } else {
        puts("open urls file failed.");
    }
    
    if(success) {

    } else {
        puts("open db file failed.");
    }

    return success;
}

static _Bool cleanup_program(fed *f) {
    
    _Bool success = true;
    
    if(f->fileUrls != NULL) {
        success = (fclose(f->fileUrls) != EOF);
    }

    if(f->conSqlite != NULL) {
        success = (success && (sqlite3_close(f->conSqlite) == SQLITE_OK));
    }

    return success;
}

#endif
