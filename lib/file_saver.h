#ifndef WEBSITE_DOWNLOADER_FILE_SAVER_H
#define WEBSITE_DOWNLOADER_FILE_SAVER_H

#include "includes.h"

void file_write(char *path, char *content) {
    FILE *log_file;
    if ((log_file = fopen(path, "w")) != 0) {
        fprintf(log_file, content);
        fclose(log_file);
    } else {
        printf("ERROR: Could not write to log file.\n");
    }
}

#endif //WEBSITE_DOWNLOADER_FILE_SAVER_H
