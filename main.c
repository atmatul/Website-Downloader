#include <stdio.h>
#include "lib/extractor.h"

int main() {
    static const char *str =
            "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "  <title>This is index page</title>\n"
                    "  <link rel=\"stylesheet\" href=\"http://w.c.c/styles/base.css\"/>\n"
                    "</head>\n"
                    "<body>\n"
                    "  <h1>This is index page</h1>\n"
                    "  Go to <a href=\"/test.html\">Test Page</a>\n"
                    "  <ul>\n"
                    "    <li>Go to <a href=\"/subdir/test_sub_1.html?h=1\">Test Sub Page 1</a></li>\n"
                    "    <li>Go to <a href=\"/subdir/test_sub_2.html\">Test Sub Page 2</a></li>\n"
                    "  </ul>\n"
                    "  <img src=\"/images/sample.png\">\n"
                    "</body>\n"
                    "</html>";

    link_extractor(str);
    return 0;
}