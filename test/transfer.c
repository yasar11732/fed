#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stdint.h>
#include "fed.h"
#include "str.h"

/*
* Mocks
*/
typedef size_t (*fn_write_callback)(const char *, size_t size, size_t nmemb, void *userdata);

typedef struct {
    void *private;
    fn_write_callback callback;
    
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

CURL *curl_easy_init_mock() {
    intptr_t p = (intptr_t)NULL;
    return (CURL*)~p;
}

CURLcode curl_easy_setopt_mock(CURL *curl, CURLoption option, ...) {
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

void curl_easy_cleanup_mock(CURL *curl) {
    (void)curl;
}

#undef curl_easy_setopt
#define curl_easy_init curl_easy_init_mock
#define curl_easy_setopt(handle, key, value) curl_easy_setopt_mock(handle, key, value)
#define curl_multi_add_handle curl_multi_add_handle_mock
#define curl_easy_cleanup curl_easy_cleanup_mock

// NOT OTHER IMPORTS BELOW HERE
#include "transfer.h"

int main(void) {
    puts("Test options are set");
    
    bool success = add_transfer(mycurlm, "https://example.com");
    assert(success);
    assert(streq(global_options.url, "https://example.com"));
    assert(notnull(global_options.private));
    assert(global_options.callback == write_cb);
    assert(global_options.timeout == 20L);
    assert(global_options.followlocation == 1L);
    assert(streq(global_options.protocols, "http,https"));
    assert(global_options.autoreferer == 1L);
    assert(global_options.maxredirs == 10L);
    assert(global_options.connecttimeout == 2000L);
    assert(streq(global_options.useragent,"feed aggregator"));
    assert(curl_multi_add_handle_count == 1);

    // TODO: Test unhappy paths too
}