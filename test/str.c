#ifdef NDEBUG
#undef NDEBUG
#endif

#include "str.h"

#include <assert.h>
#include <stdio.h>
int main()
{
    printf("TESTING streq ");
    fputs(".",stdout);
    assert(streq("",""));
    fputs(".",stdout);
    assert(!streq("a",""));
    fputs(".",stdout);
    assert(!streq("","a"));
    fputs(".",stdout);
    assert(!streq("a","b"));
    fputs(".",stdout);
    assert(streq("a","a"));
    printf("OK\r\n");

    printf("TESTING stripfilename");

#ifdef ON_WINDOWS
    // windows version
    char path[] = "C:\\users\\test\\.fed\\urls.txt";
    stripfilename(path);
    assert(streq(path, "C:\\users\\test\\.fed"));

    // check if it stops at null (first version didn't)
    char path2[] = "a\0b\\test.txt";
    stripfilename(path);
    assert(path2[3] == '\\');

    // check if it stops at max path (first version used wrong macro)
    char path3[FED_MAXPATH + 5];
    memset(path3, '\\', FED_MAXPATH+4);
    path3[FED_MAXPATH + 4] = '\0';
    stripfilename(path3);
    assert(path3[FED_MAXPATH] == '\\');
    assert(path3[FED_MAXPATH+1] == '\\');
    assert(path3[FED_MAXPATH+2] == '\\');
    assert(path3[FED_MAXPATH+3] == '\\');

#else

    // posix version
    char path[] = "/home/test/.fed/urls.txt";
    stripfilename(path);
    assert(streq(path, "/home/test/.fed"));

    // check if it stops at null (first version didn't)
    char path2[] = "a\0b/test.txt";
    stripfilename(path);
    assert(path2[3] == '/');

    char path3[FED_MAXPATH + 5];
    memset(path3, '/', FED_MAXPATH+4);
    path3[FED_MAXPATH + 4] = '\0';
    stripfilename(path3);
    assert(path3[FED_MAXPATH] == '/');
    assert(path3[FED_MAXPATH+1] == '/');
    assert(path3[FED_MAXPATH+2] == '/');
    assert(path3[FED_MAXPATH+3] == '/');
#endif

    printf("Testing copypath for overrun");
    char buf1[FED_MAXPATH + 3] = {0};
    char buf2[FED_MAXPATH + 3];

    memset(buf2,'a',sizeof(buf2));
    buf2[FED_MAXPATH+2] = '\0';

    _Bool cpresult = copypath(buf1, buf2);
    assert(!cpresult);
    assert(strlen(buf1) == (FED_MAXPATH - 1));
    assert(buf1[FED_MAXPATH] == '\0');
    assert(buf1[FED_MAXPATH+1] == '\0');

    printf("Testing strprefix");
    assert(strprefix("startsWith correct value","startsWith"));
    assert(!strprefix("does not startsWith correct value","startsWith"));
    return 0;
}
