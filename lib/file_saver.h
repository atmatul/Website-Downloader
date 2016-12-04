#ifndef WEBSITE_DOWNLOADER_FILE_SAVER_H
#define WEBSITE_DOWNLOADER_FILE_SAVER_H

#include "includes.h"

void file_write(char *path, char *content, int ishtml, int content_size) {
    FILE *log_file;
    if (!ishtml) {
        if ((log_file = fopen(path, "wb+")) != 0) {
            fprintf(log_file, "%.*s", content_size, content);
            fclose(log_file);
        } else {
            printf("ERROR: Could not write to log file.\n");
        }
    }
    else {
        if ((log_file = fopen(path, "wb+")) != 0) {
            fwrite((void *) content, 1, content_size, log_file);
            fclose(log_file);
        }
    }
}

#endif //WEBSITE_DOWNLOADER_FILE_SAVER_H
