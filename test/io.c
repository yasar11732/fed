#ifdef NDEBUG
#undef NDEBUG
#endif
// prevent io.h from getting imported too early
#define FED_IO_H

#include "fed.h"
#include "str.h"
#include <string.h>

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

void begin_test(char *msg) {
    puts(msg);

    buf[0] = '\0';

    fgets_retval = "";
    fgets_call_count = 0;

    feof_retval = 0;
    feof_call_count = 0;

    *fgetc_retval = *(int[5]){EOF, EOF, EOF, EOF, EOF};
    fgetc_index = 0;
    fgetc_call_count = 0;
}

#define fgets fgets_mock
#define feof feof_mock
#define fgetc fgetc_mock

#undef FED_IO_H
#include "io.h"

int main(void) {

    begin_test("Normal Execution");
    fgets_retval = "https://example.com";
    bool b = freadurl(buf, mockf);
    assert(b);
    assert(streq(buf, fgets_retval));

    begin_test("Fails when fgets returns null without calling feof.");
    feof_retval = 1;
    fgets_retval = NULL;
    b = freadurl(buf, mockf);
    assert(!b);
    assert(feof_call_count == 0);

    begin_test("Does not fail on exactly FED_MAXURL length line.");
    char buf2[2*FED_MAXURL];
    memset(buf2, 'a', FED_MAXURL);
    buf2[FED_MAXURL-1] = '\0';
    buf2[FED_MAXURL-2] = '\n';
    fgets_retval = buf2;
    b = freadurl(buf, mockf);
    assert(b);
    assert(streq(buf, fgets_retval));
    assert(feof_call_count == 0);

    begin_test("Does not fail on exactly FED_MAXURL and end of file.");
    memset(buf2, 'a', FED_MAXURL);
    buf2[FED_MAXURL-1] = '\0';
    fgets_retval = buf2;
    feof_retval = 1;
    
    b = freadurl(buf, mockf);
    assert(b);
    assert(streq(buf, fgets_retval));
    assert(feof_call_count == 1);

    begin_test("Fails when line is too big.");
    memset(buf2, 'a', 2*FED_MAXURL);
    buf2[(2*FED_MAXURL) - 1] = '\0';
    buf2[(2*FED_MAXURL) - 2] = '\n';
    fgets_retval = buf2;
    b = freadurl(buf, mockf);
    assert(!b);
    assert(feof_call_count == 1);

    begin_test("Skips until next line");
    fgets_retval = buf2;
    memcpy(fgetc_retval, (int[5]){'a','b','\n', EOF, EOF}, sizeof(int[5]));

    b = freadurl(buf, mockf);
    assert(fgetc_index == 3);

}