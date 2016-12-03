#include <stdio.h>
#include "lib/extractor.h"
#include "lib/downloader.h"

int main() {
    char *content;

    fetch_url("blah", &content);
    link_extractor(content);
    return 0;
}