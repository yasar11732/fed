#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h> // memset

#include "parse_args.h"

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

static void reset_fed(fed *f)
{
    memset(f, 0, sizeof(*f));
}

int main()
{

    printf("Testing parse_args ");
    fed _f, _e;
    fed *f = &_f;
    fed *e = &_e;


    reset_fed(e);

    // SANITY CHECKS

    // 1) test no args is no op
    fputs(".", stdout);
    char *test1[] = {"progname"};
    reset_fed(f);
    _Bool res = parse_args(f, _countof(test1), test1);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    // 2) test -u
    fputs(".", stdout);
    char *test2[] = {"progname","-u","/path/to/urls/file.txt"};
    reset_fed(f);
    res = parse_args(f, _countof(test2), test2);
    assert(res == true);
    assert(streq(f->pathUrls, test2[2]));

    // test -d
    fputs(".", stdout);
    char *test3[] = {"progname","-d","/path/to/database/file.db"};
    reset_fed(f);
    res = parse_args(f, _countof(test3), test3);
    assert(res == true);
    assert(streq(f->pathDB, test3[2]));

    // test -t
    fputs(".", stdout);
    char *test4[] = {"progname","-t","10"};
    reset_fed(f);
    res = parse_args(f, _countof(test4), test4);
    assert(res == true);
    assert(f->numListed == 10);

    // test -asc
    fputs(".", stdout);
    char *test5[] = {"progname","-asc"};
    reset_fed(f);
    res = parse_args(f, _countof(test5), test5);
    assert(res == true);
    assert(f->orderListed == ASC);

    // test -desc
    fputs(".", stdout);
    char *test6[] = {"progname","-desc"};
    reset_fed(f);
    res = parse_args(f, _countof(test6), test6);
    assert(res == true);
    assert(f->orderListed == DESC);

    // error conditions

    // -u without path
    fputs(".", stdout);
    char *test7[] = {"progname","-u"};
    reset_fed(f);
    res = parse_args(f, _countof(test7), test7);
    assert(res == false);

    // -d without path
    fputs(".", stdout);
    char *test8[] = {"progname","-d"};
    reset_fed(f);
    res = parse_args(f, _countof(test8), test8);
    assert(res == false);

    // -t with negative number
    fputs(".", stdout);
    char *test9[] = {"progname","-t","-10"};
    reset_fed(f);
    res = parse_args(f, _countof(test9), test9);
    assert(res == false);

    // -t with not a number
    fputs(".", stdout);
    char *test10[] = {"progname","-t","nan"};
    reset_fed(f);
    res = parse_args(f, _countof(test10), test10);
    assert(res == false);

    // -t with really big number
    fputs(".", stdout);
    char *test11[] = {"progname","-t","10000000000000000000000"};
    reset_fed(f);
    res = parse_args(f, _countof(test11), test11);
    assert(res == false);

    // test all at the same time
    fputs(".", stdout);
    char *test12[] = {"progname","-u","/path/to/urls/file.txt","-d","/path/to/database/file.db", "-t","22", "-desc"};
    reset_fed(f);
    res = parse_args(f, _countof(test12), test12);
    assert(res == true);
    assert(streq(f->pathUrls,test12[2]));
    assert(streq(f->pathDB, test12[4]));
    assert(f->numListed == 22);
    assert(f->orderListed == DESC);

    // test different orders - WARNING reusing f from above

    fputs(".", stdout);
    char *test13[] = {"progname","-u","/path/to/urls/file.txt","-d","/path/to/database/file.db", "-desc", "-t","22"};
    reset_fed(e);
    res = parse_args(e, _countof(test13), test13);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test14[] = {"progname","-u","/path/to/urls/file.txt","-desc","-d","/path/to/database/file.db", "-t","22"};
    reset_fed(e);
    res = parse_args(e, _countof(test14), test14);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test15[] = {"progname","-desc","-u","/path/to/urls/file.txt","-d","/path/to/database/file.db", "-t","22"};
    reset_fed(e);
    res = parse_args(e, _countof(test15), test15);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test16[] = {"progname","-desc","-u","/path/to/urls/file.txt","-t","22","-d","/path/to/database/file.db"};
    reset_fed(e);
    res = parse_args(e, _countof(test16), test16);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test17[] = {"progname","-desc","-t","22","-u","/path/to/urls/file.txt","-d","/path/to/database/file.db"};
    reset_fed(e);
    res = parse_args(e, _countof(test17), test17);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test18[] = {"progname","-desc","-t","22","-d","/path/to/database/file.db","-u","/path/to/urls/file.txt"};
    reset_fed(e);
    res = parse_args(e, _countof(test18), test18);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test19[] = {"progname","-desc","-d","/path/to/database/file.db","-t","22","-u","/path/to/urls/file.txt"};
    reset_fed(e);
    res = parse_args(e, _countof(test19), test19);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test20[] = {"progname","-d","/path/to/database/file.db","-desc","-t","22","-u","/path/to/urls/file.txt"};
    reset_fed(e);
    res = parse_args(e, _countof(test20), test20);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test21[] = {"progname","-d","/path/to/database/file.db","-desc","-u","/path/to/urls/file.txt","-t","22"};
    reset_fed(e);
    res = parse_args(e, _countof(test21), test21);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);

    fputs(".", stdout);
    char *test22[] = {"progname","-d","/path/to/database/file.db","-u","/path/to/urls/file.txt","-desc","-t","22"};
    reset_fed(e);
    res = parse_args(e, _countof(test22), test22);
    assert(res == true);
    assert(memcmp(f, e, sizeof(*f)) == 0);



    printf(" OK\r\n");

    return 0;
}
