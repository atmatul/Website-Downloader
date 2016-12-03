#ifndef WEBSITE_DOWNLOADER_UTIL_H
#define WEBSITE_DOWNLOADER_UTIL_H

#define NUMBER_OF_MIMETYPES 7
#define MAX_EXT_LENGTH 6

struct snode {
    char page[BUFSIZ];
    struct snode* next;
};

typedef struct snode node;

char *allowed_exts[NUMBER_OF_MIMETYPES] = {
        ".html",
        ".css",
        ".js",
        ".ico",
        ".png",
        ".jpg",
        ".jpeg"
};

int match_extension(char *path, char **ext_name) {
    int ext_match_found = -1;
    for (int i = 0; i < NUMBER_OF_MIMETYPES; i++) {
        bzero(*ext_name, MAX_EXT_LENGTH);
        if (strlen(allowed_exts[i]) < strlen(path)) {
            sprintf(*ext_name, "%s", (path + strlen(path) - strlen(allowed_exts[i])));
            if (strcmp(*ext_name, allowed_exts[i]) == 0) {
                ext_match_found = i;
                break;
            }
        }
    }
    return ext_match_found;
}

void notify_error(const char* msg) {
    printf("ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

#endif //WEBSITE_DOWNLOADER_UTIL_H
