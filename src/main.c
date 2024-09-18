
#include "parse_args.h"
#include "init_program.h"


static int usage(const char *progname)
{
    printf("USAGE: %s <OPTIONS>\r\n", progname);
    puts("OPTIONS:");
    puts("-u /path/to/urls/file.txt\tPath to file containing list of urls");
    puts("-d /path/to/database/file.db\tPath to program database");
    puts("-t num\tNumber of articles to include in result set (max 32767)");
    puts("-asc/-desc\tOrdering of results (ascending/descending)");
    return 1;
}

int main(int argc, char *argv[])
{
    fed f = {0};
    
    if(!parse_args(&f, argc, argv)) {
        return usage(argv[0]);
    }

    if(init_program(&f)) {
        
        // TODO ...

    } else if(f.fileUrls == NULL) {
        (void)fputs("Couldn't find or open urls file. "
#ifdef ON_WINDOWS
        "make sure %%USERPROFILE%%\\.fed\\urls.txt exists and readable "
#else
        "make sure ~/.fed/urls.txt exists and readable "
#endif
        "or specify another path with -u option.\r\n", stderr);
        (void)usage(argv[0]);
    }

    if(!cleanup_program(&f)) {
        (void)fprintf(stderr, "Program cleanup failed."
        "Data may be corrupted. If you encounter problems"
        ", try deleting %s and resync your feeds", f.pathDB);
    }

    return 0;
}
