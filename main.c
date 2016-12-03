#include <stdio.h>
#include "lib/extractor.h"
#include "lib/downloader.h"
#include "lib/file_saver.h"

int main() {
    char *content;

    fetch_url("blah", &content);
//    link_extractor(content);
    file_write("/Users/kunal/Desktop/NP-Project/website-downloader/index.html", content);
    return 0;
}