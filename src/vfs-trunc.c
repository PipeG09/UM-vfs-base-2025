//vfs-trunc.c

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vfs.h"

// Truncate files (remove all content but keep the file)
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s image file1 [file2...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];
    int errors = 0;

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
            errors++;
            continue;
        }
        if (inode_num < 0) {
            fprintf(stderr, "Error looking up file '%s'\n", filename);
            errors++;
            continue;
        }

        // Read inode
        struct inode file_inode;
        if (read_inode(image_path, inode_num, &file_inode) != 0) {
            fprintf(stderr, "Error reading inode for '%s'\n", filename);
            errors++;
            continue;
        }

        // Check if it's a regular file
        if ((file_inode.mode & INODE_MODE_FILE) != INODE_MODE_FILE) {
            fprintf(stderr, "'%s' is not a regular file\n", filename);
            errors++;
            continue;
        }

        // Truncate the file
        if (inode_trunc_data(image_path, &file_inode) != 0) {
            fprintf(stderr, "Error truncating file '%s'\n", filename);
            errors++;
            continue;
        }

        // Write updated inode
        if (write_inode(image_path, inode_num, &file_inode) != 0) {
            fprintf(stderr, "Error writing updated inode for '%s'\n", filename);
            errors++;
            continue;
        }

        DEBUG_PRINT("File '%s' truncated successfully\n", filename);
    }

    return errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}