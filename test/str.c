/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifdef NDEBUG
#undef NDEBUG
#endif

#include "str.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

    
    buf[0] = '\0';
    memset(&buf[1], 'X', sizeof(buf)-1);
    assert(pathncat(buf, 2, "first","second"));
    puts("Test pathncat doesn't start empty string with separator");
    assert(strprefix(buf, "first"));
    puts("Test pathncat separates components with path separator");
    assert(buf[5] == PATH_SEP);
    puts("Test pathncat null terminates");
    assert(buf[12] == '\0');

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

    puts("copy_header_value doesn't overrun destination buffer");
    char *cp_header_buf = malloc(32);
    memset(cp_header_buf, '~', 32);
    copy_header_value(cp_header_buf, 16, "ETag: 0123456789ABCDEF0123456789ABCDEF\r\n", 40);
    assert(streq(cp_header_buf, "0123456789ABCDE"));
    for(size_t i = 16; i < 32; i++) {
        assert(cp_header_buf[i] == '~');
    }

    puts("copy_header_value doesn't overrun source buffer");
    memset(cp_header_buf, '~', 32);
    copy_header_value(cp_header_buf, 32, "ETag: 0123456789ABCDEF0123456789ABCDEF\r\n", 12);
    assert(streq(cp_header_buf, "012345"));
    for(size_t i = 7; i < 32; i++) {
        assert(cp_header_buf[i] == '~');
    }
    memset(cp_header_buf, '~', 32);
    copy_header_value(cp_header_buf, 32, "X-Very-Long-Header-Value: 0123456789ABCDEF0123456789ABCDEF\r\n", 12);
    assert(streq(cp_header_buf, ""));
    
    puts("copy_header_value strips spaces");
    memset(cp_header_buf, '~', 32);
    copy_header_value(cp_header_buf, 32, "ETag: 0123456789ABCDEF \r\n", 25);
    assert(streq(cp_header_buf, "0123456789ABCDEF"));
    copy_header_value(cp_header_buf, 32, "ETag:\t0123456789ABCDEF\t\r\n", 25);
    assert(streq(cp_header_buf, "0123456789ABCDEF"));
    return 0;
}
