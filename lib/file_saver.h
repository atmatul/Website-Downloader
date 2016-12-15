#ifndef WEBSITE_DOWNLOADER_FILE_SAVER_H
#define WEBSITE_DOWNLOADER_FILE_SAVER_H

#include "includes.h"
#include "config.h"
#include "extractor.h"

/**
 * Based on the type of the file (e.g. binary or text) save the file
 * into the host hard-drive
 * @param path absolute path where the file should be saved
 * @param content the content of the http response
 * @param ishtml the flag describing the type of the file
 * @param content_size the size of the http response
 */
void file_write(char *path, char *content, int ishtml, int content_size) {
    FILE *log_file;
    if (!ishtml) {
        if ((log_file = fopen(path, "wb+")) != 0) {
            fprintf(log_file, "%.*s", content_size, content);
            fclose(log_file);
        } else {
            printf("ERROR: Could not write to file. %s\n", path);
        }
    } else {
        if ((log_file = fopen(path, "wb+")) != 0) {
            fwrite((void *) content, 1, content_size, log_file);
            fclose(log_file);
        }
    }
}

/**
 * Sanitize the save path for the remote resource. Parent and helper function
 * to file_write function. Also checks and creates the directories if not present.
 * @param config the configuration parameter containing the root save path
 * @param pagelink the link of the url to be used to maintain the directory structure
 * @param header the header of the http response
 * @param content the body of the http response
 * @param content_size the size of the body of the http response
 * @return
 */
int file_save(configuration config, char *pagelink, char *header, char *content, int content_size) {
    char filepath[BUFSIZ];
    char *ext_name = (char *) malloc(MAX_EXT_LENGTH * sizeof(char));

    sprintf(filepath, "%s%s", config.root_save_path, pagelink);
    if (filepath[strlen(filepath) - 1] == '/') {
        sprintf(filepath, "%s%s", filepath, "index.html");
    }
    if (!is_html(header) && match_extension(filepath, &ext_name) == -1) {
        sprintf(filepath, "%s%s", filepath, ".html");
    }
    free(ext_name);
    char cmd_mkdir[BUFSIZ];
    char dirpath[BUFSIZ];
    sprintf(dirpath, "%.*s", (int) (strrchr(filepath, '/') - filepath), filepath);
    if (is_valid_dir_path(dirpath, strlen(dirpath))) {
        sprintf(dirpath, "%s", config.invalid_save_path);
    }
    sprintf(cmd_mkdir, "[ -d %s ] || mkdir -p %s", dirpath, dirpath);
    if (!system(cmd_mkdir)) {
        file_write(filepath, content, is_html(header), content_size);
        return EXIT_SUCCESS;
    } else
        return EXIT_FAILURE;
}

#endif //WEBSITE_DOWNLOADER_FILE_SAVER_H
