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
    if(PATH_SEP == '\\') {
        // windows version
        char path[] = "C:\\users\\test\\.fed\\urls.txt";
        stripfilename(path);
        assert(streq(path, "C:\\users\\test\\.fed"));

        // check if it stops at null (first version didn't)
        char path2[] = "a\0b\\test.txt";
        stripfilename(path);
        assert(path2[3] == '\\');

        // check if it stops at max path (first version used wrong macro)
        char path3[MAX_PATH + 5];
        memset(path3, '\\', MAX_PATH+4);
        path3[MAX_PATH + 4] = '\0';
        stripfilename(path3);
        assert(path3[MAX_PATH] == '\\');
        assert(path3[MAX_PATH+1] == '\\');
        assert(path3[MAX_PATH+2] == '\\');
        assert(path3[MAX_PATH+3] == '\\');

    } else {
        // posix version
        char path[] = "/home/test/.fed/urls.txt";
        stripfilename(path);
        assert(streq(path, "/home/test/.fed"));

        // check if it stops at null (first version didn't)
        char path2[] = "a\0b/test.txt";
        stripfilename(path);
        assert(path2[3] == '/');

        char path3[MAX_PATH + 5];
        memset(path3, '/', MAX_PATH+4);
        path3[MAX_PATH + 4] = '\0';
        stripfilename(path3);
        assert(path3[MAX_PATH] == '/');
        assert(path3[MAX_PATH+1] == '/');
        assert(path3[MAX_PATH+2] == '/');
        assert(path3[MAX_PATH+3] == '/');
    }

    return 0;
}
