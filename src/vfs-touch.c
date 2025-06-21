#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vfs.h"

// Create empty files in the filesystem
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

        // Validate filename
        if (!name_is_valid(filename)) {
            fprintf(stderr, "Invalid filename: %s\n", filename);
            continue;
        }

        // Check if file already exists
        if (dir_lookup(image_path, filename) != 0) {
            fprintf(stderr, "File '%s' already exists\n", filename);
            continue;
        }

        // Create empty file with default permissions (rw-r-----)
        int new_inode = create_empty_file_in_free_inode(image_path, 0640);
        if (new_inode < 0) {
            fprintf(stderr, "Error creating file '%s': %s\n", filename, strerror(errno));
            continue;
        }

        // Add directory entry
        if (add_dir_entry(image_path, filename, new_inode) != 0) {
            fprintf(stderr, "Error adding directory entry for '%s'\n", filename);
            // Free the inode we just allocated
            free_inode(image_path, new_inode);
            continue;
        }

        DEBUG_PRINT("File '%s' created successfully (inode %d)\n", filename, new_inode);
    }

    return EXIT_SUCCESS;
}