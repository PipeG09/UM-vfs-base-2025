#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vfs.h"

// List directory contents in ls -l style
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s image\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];

    // Read superblock
    struct superblock sb_struct, *sb = &sb_struct;
    if (read_superblock(image_path, sb) != 0) {
        fprintf(stderr, "Error reading superblock\n");
        return EXIT_FAILURE;
    }

    // Read root directory inode
    struct inode root_inode;
    if (read_inode(image_path, ROOTDIR_INODE, &root_inode) != 0) {
        fprintf(stderr, "Error reading root directory inode\n");
        return EXIT_FAILURE;
    }

    // Print header
    printf("Inode Type Permissions Owner      Group       Blocks     Size Created             Modified            Accessed            Name\n");
    printf("===== ==== =========== ========== ========== ====== ======== =================== =================== =================== ====\n");

    // Iterate through all data blocks of root directory
    for (uint16_t i = 0; i < root_inode.blocks; i++) {
        int block_num = get_block_number_at(image_path, &root_inode, i);
        if (block_num <= 0) {
            fprintf(stderr, "Error getting block %d of root directory\n", i);
            return EXIT_FAILURE;
        }

        uint8_t data_buf[BLOCK_SIZE];
        if (read_block(image_path, block_num, data_buf) != 0) {
            fprintf(stderr, "Error reading block %d\n", block_num);
            return EXIT_FAILURE;
        }

        struct dir_entry *entries = (struct dir_entry *)data_buf;

        // Process each directory entry
        for (uint32_t j = 0; j < DIR_ENTRIES_PER_BLOCK; j++) {
            if (entries[j].inode == 0) {
                continue; // Empty entry
            }

            // Read inode for this entry
            struct inode file_inode;
            if (read_inode(image_path, entries[j].inode, &file_inode) != 0) {
                fprintf(stderr, "Error reading inode %u\n", entries[j].inode);
                continue;
            }

            // Print file information
            print_inode(&file_inode, entries[j].inode, entries[j].name);
        }
    }

    return EXIT_SUCCESS;
}