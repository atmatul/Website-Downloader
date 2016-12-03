#ifndef WEBSITE_DOWNLOADER_EXTRACTOR_H
#define WEBSITE_DOWNLOADER_EXTRACTOR_H

#include "includes.h"
#include "slre/slre.h"
#include "list.h"

#define NOT_VALID_URL -2
#define NOT_LOCAL_URL -1

int is_local_link(const char* link_url) {
    if (strlen(link_url) > 0) {
        if (link_url[0] != '/')
            return NOT_LOCAL_URL;
        else
            return EXIT_SUCCESS;
    } else {
        return NOT_VALID_URL;
    }
}

int is_html(const char* header) {
    static const char *regex = "text/html";
    struct slre_cap caps[3];
    int j = 0, str_len = strlen(header);

    if (slre_match(regex, header + j, str_len - j, caps, 3, SLRE_IGNORE_CASE) > 0) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

void link_extractor(node* lhead, const char* markup) {
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
        if (is_local_link(subpat) == 0) {
            append(lhead, subpat);
        }
        free(subpat);
        j += i;
    }
}

#endif //WEBSITE_DOWNLOADER_EXTRACTOR_H
