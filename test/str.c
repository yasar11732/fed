#ifdef NDEBUG
#undef NDEBUG
#endif

#include "str.h"

#include <assert.h>
#include <stdio.h>
int main(void)
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

#else

    // posix version
    char path[] = "/home/test/.fed/urls.txt";
    stripfilename(path);
    assert(streq(path, "/home/test/.fed"));

    // check if it stops at null (first version didn't)
    char path2[] = "a\0b/test.txt";
    stripfilename(path);
    assert(path2[3] == '/');

#endif

    puts("Testing strprefix");
    assert(strprefix("startsWith correct value","startsWith"));
    assert(!strprefix("does not startsWith correct value","startsWith"));

    puts("Testing pathncat");

    char buf[FED_MAXPATH+1];
    buf[0] = '\0';
    buf[FED_MAXPATH] = 'x'; // for checking overruns

    assert(pathncat(buf, 1, "/test/1"));
    assert(streq(buf, "/test/1"));

    puts("Test pathncat fails on too long string.");
    buf[0] = '\0';
    assert(!pathncat(buf, 2, "veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryvery","veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryvery"));
    assert(buf[FED_MAXPATH] == 'x');

    puts("Test pathncat doesn't start empty string with seperator");
    buf[0] = '\0';
    assert(pathncat(buf, 2, "first","second"));
    assert(strprefix(buf, "first"));

    puts("Test copyurl fails with too long string.");
    char copyurllongbuff[FED_MAXURL * 2];
    char copyurldestpath[FED_MAXURL + 2];

    memset(copyurllongbuff,'x',(FED_MAXURL * 2));
    copyurllongbuff[(FED_MAXURL * 2) - 1] = '\0';
    copyurldestpath[FED_MAXURL] = '~';
    assert(!copyurl(copyurldestpath, copyurllongbuff));
    assert(copyurldestpath[FED_MAXURL] == '~');

    puts("stripfilename doesn't look past FED_MAXPATH characters.");
#ifdef ON_WINDOWS
    char longpathname[] = "veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryvery\\list.txt"; 
#else
    char longpathname[] = "veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryvery/list.txt"; 
#endif
    stripfilename(longpathname);
    assert(longpathname[320] == PATH_SEP);


    return 0;
}
