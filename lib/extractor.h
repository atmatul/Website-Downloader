#ifndef WEBSITE_DOWNLOADER_EXTRACTOR_H
#define WEBSITE_DOWNLOADER_EXTRACTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slre/slre.h"

int link_extractor(const char* markup) {
    static const char *regex = "\"((https?:/)?/[^\\s/'\"<>]+[:]?[0-9]*/?[^\\s'\"<>]*)\"";
    struct slre_cap caps[3];
    int i, j = 0, str_len = strlen(markup);

    while (j < str_len &&
           (i = slre_match(regex, markup + j, str_len - j, caps, 3, SLRE_IGNORE_CASE)) > 0) {
        // extract subpattern
        char *subpat;
        subpat = (char*) malloc((caps[0].len + 1) * sizeof(char));
        memcpy(subpat, caps[0].ptr, caps[0].len);
        subpat[caps[0].len] = '\0';
        printf("Found URL: %s\n", subpat);
        free(subpat);
        j += i;
    }
    return 0;
}


#endif //WEBSITE_DOWNLOADER_EXTRACTOR_H
