#include "lib/includes.h"
#include "lib/extractor.h"
#include "lib/downloader.h"
#include "lib/file_saver.h"

int main() {
    char* host = "hyperphysics.phy-astr.gsu.edu";
    char* page = "/hbase/pber.html";
    char* root_save_path = "/Users/kunal/Desktop/NP-Project/website-downloader/public_html";

    node* head = (node *) malloc(sizeof(node));
    strcpy(head->page, page);
    head->next = NULL;

    do {
        char *content, *header;
        fetch_url(host, head->page, &header, &content);
        if (strlen(header) > 0) {
            if (!is_html(header)) {
                link_extractor(&head, content);
            }
            char filepath[BUFSIZ];
            char *ext_name;
            ext_name = (char *) malloc(MAX_EXT_LENGTH * sizeof(char));

            sprintf(filepath, "%s%s", root_save_path, head->page);
            if (filepath[strlen(filepath) - 1] == '/') {
                sprintf(filepath, "%s%s", filepath, "index.html");
            }
            if (match_extension(filepath, &ext_name) == -1) {
                sprintf(filepath, "%s%s", filepath, ".html");
            }
            char cmd_mkdir[BUFSIZ];
            char dirpath[BUFSIZ];
            sprintf(dirpath, "%.*s", (int)(strrchr(filepath, '/') - filepath), filepath);
            sprintf(cmd_mkdir, "[ -d %s ] || mkdir -p %s", dirpath, dirpath);
            if (!system(cmd_mkdir)){
                file_write(filepath, content);
                printf("Wrote: %s\n", filepath);
            } else
                notify_error("Unable to write to file.");
            free(ext_name);
        }
        if (head->next != NULL) {
            node* temp = head;
            head = head->next;
            free(temp);
        }
        free(content);
        free(header);
    } while(head != NULL);

    return 0;
}