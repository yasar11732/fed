#ifndef FED_IO_H
#define FED_IO_H
#include "fed.h"
#include <assert.h>
#include <stdio.h>

static inline bool freadurl(char *dest, FILE *f) {
    assert(notnull(dest));
    assert(notnull(f));
    
    // Ensure that the last element was not initially null
    dest[FED_MAXURL-1] = 'x';

    bool success = notnull(fgets(dest, FED_MAXURL, f));

    if(success) {
        /*
        * To ensure the integrity of the read operation:
        * 1) If the last character is non-null, the line is complete.
        * 2) If the last character is null but preceded by a newline,
        *    the line is also considered complete.
        * 3) If the last line in file was exactly FED_MAXURL-1 characters
        *    and is not newline-terminated, verify that the end of the
        *    file has been reached.
        */
        success = (dest[FED_MAXURL-1] != '\0') || (dest[FED_MAXURL-2] == '\n') || feof(f);


        /*
        * Due to reading a partial line, additional characters
        * must be consumed until the end-of-line delimiter is
        * reached, ensuring proper alignment for the subsequent line.
        */
        if(!success) {
            int c;
            do {
                c = fgetc(f);
            } while((c > 0) && (c !='\n'));
        }
    }

    return success;

}

#endif