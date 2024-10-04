#ifdef NDEBUG
#undef NDEBUG
#endif

// prevent premature inclusion of transer.h
#define FED_TRANSFER_H 

#include <assert.h>
#include <stdint.h>
#include "fed.h"
#include "str.h"
#include "init_program.h"
#include "transfer_mem.h"
#include <string.h>

fed local_fed;

char buf[FED_MAXURL+1];
FILE *mockf = (FILE*)((char*)NULL+1);

int feof_retval = 0;
int feof_call_count = 0;

int feof_mock(FILE *stream) {
    (void)stream;
    feof_call_count++;
    return feof_retval;
}


static char *fgets_retval = "";
int fgets_call_count = 0;

char *fgets_mock(char *s, int n, FILE *stream) {
    
    (void)stream;

    fgets_call_count++;
    if(fgets_retval == NULL)
        return NULL;

    size_t len = strlen(fgets_retval);
    
    if(!(len < (size_t)n)) {
        len = n-1;
    }

    memcpy(s, fgets_retval, len);
    s[len] = '\0';
    return s;
}

int fgetc_retval[5] = {EOF, EOF, EOF, EOF, EOF};
int fgetc_index = 0;
int fgetc_call_count = 0;

int fgetc_mock(FILE *stream) {
    (void)stream;
    fgetc_call_count++;
    return fgetc_retval[fgetc_index++];
}

/*
* Mocks
*/
typedef size_t (*fn_write_callback)(const char *, size_t size, size_t nmemb, void *userdata);
typedef size_t (*fn_header_callback)(char *buffer, size_t size, size_t nitems, void *userdata);

typedef struct {
    void *private;
    void *writedata;
    void *headerdata;

    fn_write_callback callback;
    fn_header_callback header_cb;
    
    long timeout;
    long followlocation;
    long autoreferer;
    long maxredirs;
    long connecttimeout;
    
    char protocols[50];
    char useragent[256];
    char url[FED_MAXURL];
} my_curl_options_t;

my_curl_options_t global_options;

CURL *curl_easy_init_retval;
CURL *curl_easy_init_mock(void) {
    return curl_easy_init_retval;
}

CURLcode curl_easy_setopt_retval;

CURLcode curl_easy_setopt_mock(CURL *curl, CURLoption option, ...) {

    if(curl_easy_setopt_retval != CURLE_OK) {
        return curl_easy_setopt_retval;
    }

    (void)curl;
    va_list arg;
    va_start(arg, option);

    switch(option) {
        case CURLOPT_URL:
            assert(copyurl(global_options.url, va_arg(arg, char *)));
            break;
        case CURLOPT_PRIVATE:
            global_options.private = va_arg(arg, void *);
            break;
        case CURLOPT_WRITEFUNCTION:
            global_options.callback = va_arg(arg, fn_write_callback);
            break;
        case CURLOPT_TIMEOUT:
            global_options.timeout = va_arg(arg, long);
            break;
        case CURLOPT_FOLLOWLOCATION:
            global_options.followlocation = va_arg(arg, long);
            break;
        case CURLOPT_PROTOCOLS_STR:
            strcpy(global_options.protocols, va_arg(arg, char *));
            break;
        case CURLOPT_AUTOREFERER:
            global_options.autoreferer = va_arg(arg, long);
            break;
        case CURLOPT_MAXREDIRS:
            global_options.maxredirs = va_arg(arg, long);
            break;
        case CURLOPT_CONNECTTIMEOUT_MS:
            global_options.connecttimeout = va_arg(arg, long);
            break;
        case CURLOPT_USERAGENT:
            strcpy(global_options.useragent, va_arg(arg, char *));
            break;
        case CURLOPT_WRITEDATA:
            global_options.writedata = va_arg(arg, void *);
            break;
        case CURLOPT_HEADERFUNCTION:
            global_options.header_cb = va_arg(arg, fn_header_callback);
            break;
        case CURLOPT_HEADERDATA:
            global_options.headerdata = va_arg(arg, void *);
            break;
        default:
            return CURLE_BAD_FUNCTION_ARGUMENT;
    }

    va_end(arg);
    return CURLE_OK;
}

int curl_multi_add_handle_count = 0;
CURLM *mycurlm = (CURLM *)~(intptr_t)NULL;

CURLMcode curl_multi_add_handle_mock(CURLM *mh, CURL *eh) {
    (void)eh;
    curl_multi_add_handle_count++;
    
    if(mh == mycurlm && notnull(eh))
        return CURLM_OK;
    else
        return CURLM_BAD_FUNCTION_ARGUMENT;
}

int curl_easy_cleanup_calls = 0;
void curl_easy_cleanup_mock(CURL *curl) {
    (void)curl;
    curl_easy_cleanup_calls++;
}

/*
* Resets all global variables before a test 
*/
void begin_test(char *msg) {
    puts(msg);

    if(notnull(local_fed.conSqlite)) {
        sqlite3_close(local_fed.conSqlite);
    }
    init_fed(&local_fed);
    strcpy(local_fed.pathDB,":memory:");
    sqlite3_open(local_fed.pathDB, &local_fed.conSqlite);
    initialize_db(&local_fed);
    local_fed.fileUrls = (FILE*)(((char*)NULL)+1);
    local_fed.mh = mycurlm;

    alloc_mask = 0xFFFFFFFFul;

    buf[0] = '\0';

    fgets_retval = "";
    fgets_call_count = 0;

    feof_retval = 0;
    feof_call_count = 0;

    *fgetc_retval = *(int[5]){EOF, EOF, EOF, EOF, EOF};
    fgetc_index = 0;
    fgetc_call_count = 0;

    intptr_t p = (intptr_t)NULL;
    curl_easy_init_retval = (CURL*)~p;
    curl_easy_setopt_retval = CURLE_OK;
    curl_easy_cleanup_calls = 0;
}

#undef curl_easy_setopt
#define curl_easy_init curl_easy_init_mock
#define curl_easy_setopt(handle, key, value) curl_easy_setopt_mock(handle, key, value)
#define curl_multi_add_handle curl_multi_add_handle_mock
#define curl_easy_cleanup curl_easy_cleanup_mock
#define fgets fgets_mock
#define feof feof_mock
#define fgetc fgetc_mock

// NOT OTHER IMPORTS BELOW HERE
#undef FED_TRANSFER_H
#include "transfer.h"

int main(void) {
    puts("add_transfer sets options.");
    
    begin_test("add_transfer sets options.");
    fgets_retval = "https://example.com";
    bool success = add_transfer(&local_fed);
    assert(success);
    assert(streq(global_options.url, "https://example.com"));
    assert(notnull(global_options.private));
    assert(notnull(global_options.writedata));
    assert(global_options.private == global_options.writedata);
    assert(global_options.callback == write_cb);
    assert(global_options.timeout == 20L);
    assert(global_options.followlocation == 1L);
    assert(streq(global_options.protocols, "http,https"));
    assert(global_options.autoreferer == 1L);
    assert(global_options.maxredirs == 10L);
    assert(global_options.connecttimeout == 2000L);
    assert(streq(global_options.useragent,"feed aggregator"));
    assert(curl_multi_add_handle_count == 1);
    assert(global_options.header_cb == header_cb);
    assert(global_options.headerdata == global_options.writedata);

    begin_test("write_cb writes up to FED_MAXDATA bytes.");
    transfer_t *t = new_transfer();
    assert(notnull(t));

    char buf_writecb[5] = {0x1A, 0x2E, 0x3A, 0x4E, 0x5E };
    size_t total_written = 0;
    size_t chunk_written = 0;
    do {
        chunk_written = global_options.callback(buf_writecb, 1, sizeof(buf_writecb), t);
        total_written += chunk_written;
    } while(chunk_written == sizeof(buf_writecb));

    assert(t->cbData == FED_MAXDATA);
    assert(global_options.callback(buf_writecb, 1, sizeof(buf_writecb), t) == 0);
    for(size_t i = 0; i < t->cbData; i++) {
        assert(t->data[i] == buf_writecb[i % sizeof(buf_writecb)]);
    }

    begin_test("freadurl reads url");
    fgets_retval = "https://example.com";
    bool b = freadurl(buf, mockf);
    assert(b);
    assert(streq(buf, fgets_retval));

    begin_test("freadurl fails when fgets returns null without calling feof.");
    feof_retval = 1;
    fgets_retval = NULL;
    b = freadurl(buf, mockf);
    assert(!b);
    assert(feof_call_count == 0);

    begin_test("freadurl does not fail on exactly FED_MAXURL length line.");
    char buf2[2*FED_MAXURL];
    memset(buf2, 'a', FED_MAXURL);
    buf2[FED_MAXURL-1] = '\0';
    buf2[FED_MAXURL-2] = '\n';
    fgets_retval = buf2;
    b = freadurl(buf, mockf);
    assert(b);
    // assert(streq(buf, fgets_retval));
    assert(feof_call_count == 0);

    begin_test("freadurl does not fail on exactly FED_MAXURL and end of file.");
    memset(buf2, 'a', FED_MAXURL);
    buf2[FED_MAXURL-1] = '\0';
    fgets_retval = buf2;
    feof_retval = 1;
    
    b = freadurl(buf, mockf);
    assert(b);
    assert(streq(buf, fgets_retval));
    assert(feof_call_count == 1);

    begin_test("freadurl fails when line is too big.");
    memset(buf2, 'a', 2*FED_MAXURL);
    buf2[(2*FED_MAXURL) - 1] = '\0';
    buf2[(2*FED_MAXURL) - 2] = '\n';
    fgets_retval = buf2;
    b = freadurl(buf, mockf);
    assert(!b);
    assert(feof_call_count == 1);

    begin_test("freadurl removes newline from fgets result");
    fgets_retval = "http://www.example.com/\n";
    b = freadurl(buf, mockf);
    assert(b);
    assert(streq(buf, "http://www.example.com/"));

    fgets_retval = "http://www.example.com/\r\n";
    b = freadurl(buf, mockf);
    assert(b);
    assert(streq(buf, "http://www.example.com/"));

    begin_test("freadurl skips until next line if current line is too big.");
    fgets_retval = buf2;
    memcpy(fgetc_retval, (int[5]){'a','b','\n', EOF, EOF}, sizeof(int[5]));

    b = freadurl(buf, mockf);
    assert(fgetc_index == 3);

    begin_test("add_transfer frees transfer on failure");
    curl_easy_init_retval = NULL;
    b = add_transfer(&local_fed);
    assert(!b);
    assert(alloc_mask == 0xFFFFFFFF);

    begin_test("add_transfer cleans up easy handle on failure");
    curl_easy_setopt_retval = CURLE_BAD_FUNCTION_ARGUMENT;
    b = add_transfer(&local_fed);
    assert(!b);
    assert(curl_easy_cleanup_calls > 0);

    begin_test("add_transfer does not free NULL transfer");
    alloc_mask = 0;
    b = add_transfer(&local_fed);
    assert(!b);
    // transfer freeing function does assertion for invalid pointers

    begin_test("add_transfers adds transfers");
    add_transfers(&local_fed);
    assert(local_fed.runningHandles == FED_MAXPARALLEL);

    begin_test("add_transfers does not add transfers if end of file is reached");
    feof_retval = 1;
    add_transfers(&local_fed);
    assert(local_fed.runningHandles == 0);

}
