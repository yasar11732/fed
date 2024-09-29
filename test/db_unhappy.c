#ifdef NDEBUG
#undef NDEBUG
#endif

#define FED_DB_H

#include "fed.h"
#include "str.h"
#include "sqlite3.h"
#include <assert.h>

typedef int (*exec_callback)(void*,int,char**,char**);
char *sqlite3_exec_error = NULL;
int sqlite3_exec_return = SQLITE_OK;
const FILE *expected_stream = NULL;
const char *expected_format = NULL;
const char *expected_errmsg = NULL;

int fprintf_mock_calls = 0;
int sqlite3_exec_mock_calls = 0;
int sqlite3_free_mock_calls = 0;


// this also validates number of args at compilation time.
void fprintf_mock(FILE *stream, const char *format, const char *errmsg) {
    
    fprintf_mock_calls++;

    assert(stream == expected_stream);
    assert(streq(format, expected_format));
    assert(streq(errmsg, expected_errmsg));

}

int sqlite3_exec_mock( sqlite3* con, const char *sql, exec_callback cb, void *arg_cb,char **errmsg) {
    (void)con;
    (void)sql;
    (void)cb;
    (void)(arg_cb);

    sqlite3_exec_mock_calls++;
    *errmsg = sqlite3_exec_error;
    return sqlite3_exec_return;
}

void sqlite3_free_mock(void *ptr) {
    sqlite3_free_mock_calls++;
    (void)ptr;
}

void begin_test(const char *msg) {
    puts(msg);

    sqlite3_exec_error = NULL;
    expected_stream = NULL;
    expected_format = NULL;
    expected_errmsg = NULL;
    sqlite3_exec_return = SQLITE_OK;

    fprintf_mock_calls = 0;
    sqlite3_exec_mock_calls = 0;
    sqlite3_free_mock_calls = 0;
}



#define sqlite3_exec sqlite3_exec_mock
#define fprintf fprintf_mock
#define sqlite3_free sqlite3_free_mock

#undef FED_DB_H
#include "db.h"

int main(void) {
    
    begin_test("Test error message printed and freed");

    sqlite3_exec_return = SQLITE_NOMEM;
    sqlite3_exec_error = "Out of memory";
    
    expected_stream = stderr;
    expected_format = "sqlite3 error: %s";
    expected_errmsg = sqlite3_exec_error;
    fed f;
    init_fed(&f);
    f.conSqlite = (sqlite3*)5;
    bool b = initialize_db(&f);
    
    assert(!b);
    assert(fprintf_mock_calls > 0);
    assert(sqlite3_exec_mock_calls > 0);
    assert(sqlite3_free_mock_calls > 0);


}