/*
* Order of imports is very important here.
* We don't want to override standard library
* definitions with our mocks.
*/

// important for assert to work
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>

#include "fed.h"
#include "str.h"
#include "db.h"
#include <stdint.h> // for intptr_t
#include <string.h>
#include <curl/curl.h>

/*
* =========================
* GLOBALS
* =========================
*/
#define MOCK_SIZE 32
// mock file system data (null terminated)
char *fs[MOCK_SIZE];
// mock environment data (null terminated)
char *env[MOCK_SIZE]; 

int open_file_count = 0;
int fopen_call_count = 0;
int fclose_call_count = 0;
int get_env_call_count = 0;
fed local_f;

CURLcode global_init_ret;
int global_init_call_count = 0;
int global_cleanup_call_count = 0;

int sqlite3_open_call_count = 0;
int sqlite3_close_call_count = 0;

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
        char *e = env[i];

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
        
        char *mockfile = fs[i];

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
    
    return 0;
}

CURLcode curl_global_init_mock(long flags) {
    global_init_call_count++;
    return global_init_ret;
}

void curl_global_cleanup_mock(void) {
    global_cleanup_call_count++;
}

_Bool initialize_db_mock(fed *f) {
    return true;
}

CURLcode sqlite3_open_mock(char *filename, sqlite3 **ppDB) {
    sqlite3_open_call_count++;
    intptr_t p = (intptr_t)NULL;
    *ppDB = (sqlite3*)~p;
    return CURLE_OK;
}

int sqlite3_close_mock(sqlite3 *p) {
    sqlite3_close_call_count++;
    return SQLITE_OK;
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

#include "init_program.h"

/*
* Reset all global variables and 
* print name of the test
*/
static void begin_test(char *name) {

    int i;
    
    for(i = 0; i < MOCK_SIZE; i++) {
        fs[i] = NULL;
        env[i] = NULL;
    }
    memset(&local_f, 0, sizeof(local_f));
    
    global_init_call_count = 0;
    global_cleanup_call_count = 0;
    global_init_ret = CURLE_OK;

    sqlite3_open_call_count = 0;
    sqlite3_close_call_count = 0;

    fopen_call_count = 0;
    fclose_call_count = 0;
    open_file_count = 0;
    get_env_call_count = 0;
    puts(name);

};

int main() {

    begin_test("init_program zero initializes");
    init_program(&local_f);
    fed g = {0};
    assert(memcmp(&local_f, &g, sizeof(g)) == 0);

    begin_test("freadable test non existing");
    fs[0] = "/a.txt";
    fs[1] = "/b.txt";
    _Bool b = freadable("/c.txt");
    assert(b == false);
    assert(fopen_call_count == 1);
    assert(fclose_call_count == 0);
    assert(open_file_count == 0);

    begin_test("freadable test existing");
    fs[0] = "/a.txt";
    fs[1] = "/b.txt";
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
    env[0] = "USERPROFILE=C:\\users\\test";
    fs[0] = "C:\\users\\test\\.fed\\urls.txt";
    b = locate_urls_file(&local_f);
    assert(b == true);
    assert(streq(local_f.pathUrls, fs[0]));

    begin_test("locate_urls_file in appdata directory");
    env[0] = "APPDATA=C:\\users\\test\\appdata\\roaming";
    fs[0] = "C:\\users\\test\\appdata\\roaming\\.fed\\urls.txt";
    b = locate_urls_file(&local_f);
    assert(b == true);
    assert(streq(local_f.pathUrls, fs[0]));
#else    
    begin_test("locate_urls_file in home directory");
    env[0] = "HOME=/home/test";
    fs[0] = "/home/test/.fed/urls.txt";
    b = locate_urls_file(&local_f);
    assert(b == true);
    assert(streq(local_f.pathUrls, fs[0]));

    begin_test("locate_urls_file in /var/lib/fed directory");
    fs[0] = "/var/lib/fed/urls.txt";
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
    fs[0] = "test/path/urls.txt";
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

}