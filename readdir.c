/**
 * readdir.c
 *
 * Demonstrates reading directory contents.
 */

#include <dirent.h>
#include <stdio.h>

int main(void) {
    DIR *directory;
    if ((directory = opendir(".")) == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        printf("-> %s\n", entry->d_name);
    }

    closedir(directory);

    return 0;
}
