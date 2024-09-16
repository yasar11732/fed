
#include "parse_args.h"
#include "init_program.h"


static int usage(const char *progname)
{
    printf("USAGE: %s <OPTIONS>\r\n", progname);
    puts("OPTIONS:");
    puts("-u /path/to/urls/file.txt\tPath to file containing list of urls");
    puts("-d /path/to/database/file.db\tPath to program database");
    puts("-t num\tNumber of recent articles to show (max 32767)");
    puts("-asc/-desc\tOrder articles in ascending/descending order");
    return 1;
}

int main(int argc, char *argv[])
{
    fed f;
    
    if(!parse_args(&f, argc, argv)) {
        return usage(argv[0]);
    }

    if(init_program(&f)) {

    }

    if(!cleanup_program(&f)) {
        (void)fprintf(stderr, "Program cleanup failed."
        "Data may be corrupted. If you encounter problems"
        ", try deleting %s and resync your feeds", f.pathDB);
    }

    return 0;
}
