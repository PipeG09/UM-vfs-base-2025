#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vfs.h"

// Display file contents
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s image file1 [file2...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];

    // Verify image
    struct superblock sb_struct, *sb = &sb_struct;
    if (read_superblock(image_path, sb) != 0) {
        fprintf(stderr, "Error reading superblock\n");
        return EXIT_FAILURE;
    }

    // Process each file
    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];

        // Look up file in directory
        int inode_num = dir_lookup(image_path, filename);
        if (inode_num == 0) {
            fprintf(stderr, "File '%s' not found\n", filename);
            continue;
        }
        if (inode_num < 0) {
            fprintf(stderr, "Error looking up file '%s'\n", filename);
            continue;
        }

        // Read inode
        struct inode file_inode;
        if (read_inode(image_path, inode_num, &file_inode) != 0) {
            fprintf(stderr, "Error reading inode for '%s'\n", filename);
            continue;
        }

        // Check if it's a regular file
        if ((file_inode.mode & INODE_MODE_FILE) != INODE_MODE_FILE) {
            fprintf(stderr, "'%s' is not a regular file\n", filename);
            continue;
        }

        // Read and display file contents
        if (file_inode.size > 0) {
            uint8_t *buffer = malloc(file_inode.size);
            if (!buffer) {
                fprintf(stderr, "Error allocating memory for file '%s'\n", filename);
                continue;
            }

            ssize_t bytes_read = inode_read_data(image_path, inode_num, buffer, file_inode.size, 0);
            if (bytes_read < 0) {
                fprintf(stderr, "Error reading data from file '%s'\n", filename);
                free(buffer);
                continue;
            }

            // Write to stdout
            if (fwrite(buffer, 1, bytes_read, stdout) != (size_t)bytes_read) {
                fprintf(stderr, "Error writing to stdout\n");
                free(buffer);
                return EXIT_FAILURE;
            }

            free(buffer);
        }
    }

    return EXIT_SUCCESS;
}