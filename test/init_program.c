/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

// important for assert to work
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>

#include "fed.h"
#include "str.h"
#include <stdlib.h>
#include <stdint.h> // for intptr_t
#include <string.h>
#include <curl/curl.h>

// to prevent db.h to be included later
#define FED_DB_H
/*
* =========================
* GLOBALS
* =========================
*/
#define MOCK_SIZE 32
// mock file system data (null terminated)
char *fs_mocks[MOCK_SIZE];
// mock environment data (null terminated)
char *env_mocks[MOCK_SIZE]; 

int open_file_count = 0;
int fopen_call_count = 0;
int fclose_call_count = 0;
int fclose_return_value = 0;
int get_env_call_count = 0;
fed local_f;

CURLcode global_init_ret;
int global_init_call_count = 0;
int global_cleanup_call_count = 0;

int sqlite3_open_call_count = 0;
int sqlite3_close_call_count = 0;
int sqlite3_close_return_value = SQLITE_OK;

/*
* ======================
* MOCK PROCEDURES
* ======================
*/
char *getenv_mock(const char *name)
{
    get_env_call_count++;
    size_t len = strlen(name);

    int i;
    for(i = 0; i < MOCK_SIZE; i++) {
        char *e = env_mocks[i];

        if(e == NULL) {
            break;
        }
        
        if((strncmp(name, e, len)) == 0 && (e[len] == '=')) {
            return &e[len+1];
        }
    }

    return NULL;
}

FILE *fopen_mock(const char * restrict filename, const char * restrict mode)
{
    (void)mode;

    fopen_call_count++;
    int i;
    for(i = 0; i < MOCK_SIZE; i++) {
        
        char *mockfile = fs_mocks[i];

        if(mockfile == NULL) {
            break;
        }

        if(streq(filename, mockfile)) {
            open_file_count++;
            intptr_t p = (intptr_t)NULL;
            return (FILE *)~p;
        }

    }

    return (FILE *)NULL;
}

int fclose_mock(FILE *stream)
{
    (void)stream;
    fclose_call_count++;
    
    if(stream != NULL)
        open_file_count--;
    
    return fclose_return_value;
}

CURLcode curl_global_init_mock(long flags) {
    (void)flags;
    global_init_call_count++;
    return global_init_ret;
}

void curl_global_cleanup_mock(void) {
    global_cleanup_call_count++;
}

int curl_multi_cleanup_calls = 0;
CURLMcode curl_multi_cleanup_retval = CURLM_OK;
CURLMcode curl_multi_cleanup_mock(CURLM *multi_handle)
{
    (void)multi_handle;
    curl_multi_cleanup_calls++;
    return curl_multi_cleanup_retval;
}

bool initialize_db_mock(fed *f) {
    (void)f;
    return true;
}

CURLcode sqlite3_open_mock(char *filename, sqlite3 **ppDB) {
    (void)filename;

    sqlite3_open_call_count++;
    intptr_t p = (intptr_t)NULL;
    *ppDB = (sqlite3*)~p;
    return CURLE_OK;
}

int sqlite3_close_mock(sqlite3 *p) {
    (void)p;
    sqlite3_close_call_count++;
    return sqlite3_close_return_value;
}
/*
* ===========================================
* WARNING! NO STDLIB IMPORTS BELOW THIS POINT
* ===========================================
*/
#define getenv getenv_mock
#define fopen fopen_mock
#define fclose fclose_mock
#define curl_global_init curl_global_init_mock
#define curl_global_cleanup curl_global_cleanup_mock
#define initialize_db initialize_db_mock
#define sqlite3_open sqlite3_open_mock
#define sqlite3_close sqlite3_close_mock
#define curl_multi_cleanup curl_multi_cleanup_mock

#include "init_program.h"

/*
* Reset all global variables and 
* print name of the test
*/
static void begin_test(char *name) {

    puts(name);

    int i;
    
    for(i = 0; i < MOCK_SIZE; i++) {
        fs_mocks[i] = NULL;
        env_mocks[i] = NULL;
    }

    init_fed(&local_f);

    global_init_call_count = 0;
    global_cleanup_call_count = 0;
    global_init_ret = CURLE_OK;

    sqlite3_open_call_count = 0;
    sqlite3_close_call_count = 0;
    sqlite3_close_return_value = SQLITE_OK;

    fopen_call_count = 0;
    fclose_call_count = 0;
    fclose_return_value = 0;
    open_file_count = 0;
    get_env_call_count = 0;

    curl_multi_cleanup_calls = 0;
    curl_multi_cleanup_retval = CURLM_OK;
}

int main(void) {

    begin_test("freadable test non existing");
    fs_mocks[0] = "/a.txt";
    fs_mocks[1] = "/b.txt";
    bool b = freadable("/c.txt");
    assert(b == false);
    assert(fopen_call_count == 1);
    assert(fclose_call_count == 0);
    assert(open_file_count == 0);

    begin_test("freadable test existing");
    fs_mocks[0] = "/a.txt";
    fs_mocks[1] = "/b.txt";
    b = freadable("/a.txt");
    assert(b == true);
    assert(fopen_call_count == 1);
    assert(fclose_call_count == 1);
    assert(open_file_count == 0);

    begin_test("locate_urls_file preset location");
    strcpy(local_f.pathUrls, "/path/to/urls.txt");
    b = locate_urls_file(&local_f);
    assert(b == true);
    assert(fopen_call_count == 0);
    assert(fclose_call_count == 0);
    assert(open_file_count == 0);
    assert(get_env_call_count == 0);
    assert(streq(local_f.pathUrls, "/path/to/urls.txt"));

#ifdef ON_WINDOWS
    begin_test("locate_urls_file in userprofile directory");
    env_mocks[0] = "USERPROFILE=C:\\users\\test";
    fs_mocks[0] = "C:\\users\\test\\.fed\\urls.txt";
    b = locate_urls_file(&local_f);
    assert(b == true);
    assert(streq(local_f.pathUrls, fs_mocks[0]));

    begin_test("locate_urls_file in appdata directory");
    env_mocks[0] = "APPDATA=C:\\users\\test\\appdata\\roaming";
    fs_mocks[0] = "C:\\users\\test\\appdata\\roaming\\.fed\\urls.txt";
    b = locate_urls_file(&local_f);
    assert(b == true);
    assert(streq(local_f.pathUrls, fs_mocks[0]));
#else    
    begin_test("locate_urls_file in home directory");
    env_mocks[0] = "HOME=/home/test";
    fs_mocks[0] = "/home/test/.fed/urls.txt";
    b = locate_urls_file(&local_f);
    assert(b == true);
    assert(streq(local_f.pathUrls, fs_mocks[0]));

    begin_test("locate_urls_file in /var/lib/fed directory");
    fs_mocks[0] = "/var/lib/fed/urls.txt";
    b = locate_urls_file(&local_f);
    assert(b == true);
    assert(streq(local_f.pathUrls, "/var/lib/fed/urls.txt"));
#endif

    begin_test("locate_urls_file not found");
    b = locate_urls_file(&local_f);
    assert(b == false);
    assert(streq(local_f.pathUrls, ""));
    
    begin_test("init/cleanup");
    strcpy(local_f.pathUrls,"test/path/urls.txt");
    fs_mocks[0] = "test/path/urls.txt";
    b = init_program(&local_f);
    assert(b == true);
    assert(global_init_call_count == 1);
    assert(sqlite3_open_call_count == 1);
    assert(local_f.fileUrls != NULL);
    assert(local_f.conSqlite != NULL);
    b = cleanup_program(&local_f);
    assert(b == true);
    assert(global_cleanup_call_count == 1);
    assert(sqlite3_close_call_count == 1);
    assert(open_file_count == 0);

    begin_test("find_in_env rejects long pathname");
#ifdef ON_WINDOWS
    env_mocks[0] = "USERPROFILE=C:\\users\\veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryverylong";
    fs_mocks[0] = "C:\\users\\veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryverylong\\.fed\\urls.txt";
    assert(find_in_env(&local_f,"USERPROFILE") == false);
#else
    env_mocks[0] = "HOME=/home/veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryverylong";
    fs_mocks[0] = "/home/veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryverylong/.fed/urls.txt";
    assert(find_in_env(&local_f,"HOME") == false);
#endif
    
    begin_test("find_in_env doesn't nonexisting file.");

#ifdef ON_WINDOWS
    env_mocks[0] = "USERPROFILE=C:\\users\\test";
    assert(find_in_env(&local_f, "USERPROFILE") == false);
#else
    env_mocks[0] = "HOME=/home/test";
    assert(find_in_env(&local_f, "HOME") == false);
#endif

    begin_test("locate_db_file is noop with preset location");
    strcpy(local_f.pathDB,"/home/test/.fed/fed.db");
    b = locate_db_file(&local_f);
    assert(b);
    assert(streq(local_f.pathDB, "/home/test/.fed/fed.db"));
    assert(fopen_call_count == 0);
    assert(get_env_call_count == 0);

    begin_test("locate_db_file fails if both pathUrls and pathDB is empty");
    b = locate_db_file(&local_f);
    assert(!b);
    assert(streq(local_f.pathDB,""));
    assert(fopen_call_count == 0);
    assert(get_env_call_count == 0);

    begin_test("open_db_file fails if both pathUrls and pathDB is empty.");
    b = open_db_file(&local_f);
    assert(!b);

    begin_test("cleanup_program doesn't close null urls file.");
    (void)cleanup_program(&local_f);
    assert(fclose_call_count == 0);

    begin_test("cleanup_program doesn't close null sqlite3 conn.");
    (void)cleanup_program(&local_f);
    assert(sqlite3_close_call_count == 0);

    begin_test("cleanup_program fails if fclose fails but sqlite3_close doesn't");
    local_f.conSqlite = (sqlite3*)((char*)NULL+1);
    local_f.fileUrls = (FILE*)((char*)NULL+1);
    fclose_return_value = -1;
    assert(!cleanup_program(&local_f));

    begin_test("cleanup_program fails if sqlite3_close fails but fclose doesn't");
    local_f.conSqlite = (sqlite3*)((char*)NULL+1);
    local_f.fileUrls = (FILE*)((char*)NULL+1);
    sqlite3_close_return_value = SQLITE_BUSY;
    assert(!cleanup_program(&local_f));

    begin_test("cleanup_program succeed if sqlite3_close and fclose succeed.");
    local_f.conSqlite = (sqlite3*)((char*)NULL+1);
    local_f.fileUrls = (FILE*)((char*)NULL+1);
    assert(cleanup_program(&local_f));
    assert(fclose_call_count == 1);
    assert(sqlite3_close_call_count == 1);

    begin_test("cleanup_program fails if curl_multi_cleanup fails.");
    local_f.conSqlite = (sqlite3*)((char*)NULL+1);
    local_f.fileUrls = (FILE*)((char*)NULL+1);
    local_f.mh = (CURLM *)((char*)NULL+1);
    curl_multi_cleanup_retval = CURLM_BAD_HANDLE;
    assert(!cleanup_program(&local_f));
    assert(curl_multi_cleanup_calls > 0);

    begin_test("cleanup_program calls curl_multi_cleanup even if earlier steps fails.");
    local_f.fileUrls = (FILE*)((char*)NULL+1);
    fclose_return_value = -1;

    local_f.mh = (CURLM *)((char*)NULL+1);

    assert(!cleanup_program(&local_f));
    assert(curl_multi_cleanup_calls > 0);

    begin_test("init_program fails if open_urls_file fails");
    fclose_return_value = -1;
    assert(!init_program(&local_f));

}
