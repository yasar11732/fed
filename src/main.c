
#include "fed.h"
#include "parse_args.h"
#include "init_program.h"
#include "main_loop.h"

#include <stdio.h>
#include <stddef.h>


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
    fed f;
    init_fed(&f);
    
    if(!parse_args(&f, argc, argv)) {
        return usage(argv[0]);
    }

    if(init_program(&f)) {
        
        add_transfers(&f);
        main_loop(&f);

    } else if(f.fileUrls == NULL) {
        (void)fputs("Unable to locate or open the URLs file. Ensure that "
#ifdef ON_WINDOWS
        "%%USERPROFILE%%\\.fed\\urls.txt "
#else
        "~/.fed/urls.txt "
#endif
        "exists and is readable, or specify an alternative path using the -u option.\r\n", stderr);
        (void)usage(argv[0]);
    }

    if(!cleanup_program(&f)) {
        (void)fprintf(stderr, "Program cleanup failed."
        "Data may be corrupted. If you encounter problems"
        ", try deleting %s and resync your feeds", f.pathDB);
    }

    return 0;
}
