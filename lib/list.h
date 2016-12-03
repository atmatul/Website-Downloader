#ifndef WEBSITE_DOWNLOADER_LIST_H
#define WEBSITE_DOWNLOADER_LIST_H

#include "includes.h"

void append(node** lhead, const char* url) {
    node* right = *lhead;
    node* temp = (node *) malloc(sizeof(node));

    strcpy(temp->page, url);
    temp->next = NULL;

    if (right == NULL) {
        *lhead = temp;
    } else {
        while (right->next != NULL) {
            right = right->next;
        }
        right->next = temp;
    }
}

void add(node* lnode, const char* url) {
    node* temp = (node *) malloc(sizeof(node));
    strcpy(temp->page, url);

    if (lnode == NULL) {
        lnode = temp;
        lnode->next = NULL;
    } else {
        temp->next = lnode->next;
        lnode->next = temp;
    }
}

void progress(node** lhead) {
    node* next = *lhead;

    if (*lhead != NULL) {
        *lhead = (*lhead)->next;
    }

    free(next);
}

#endif //WEBSITE_DOWNLOADER_LIST_H
