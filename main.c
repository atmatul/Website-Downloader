#include "lib/extractor.h"
#include "lib/downloader.h"
#include "lib/file_saver.h"

int main() {
    char *content;
    char *header;

    fetch_url("blah", &header, &content);
//    link_extractor(content);
//    file_write("/Users/kunal/Desktop/NP-Project/website-downloader/index.html", content);
    free(content);
    free(header);
    return 0;
}