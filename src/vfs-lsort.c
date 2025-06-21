#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vfs.h"

// Structure to hold file information for sorting
struct file_info {
    uint32_t inode_num;
    char name[FILENAME_MAX_LEN];
    struct inode inode_data;
};

// Comparison function for qsort
int compare_names(const void *a, const void *b) {
    const struct file_info *file_a = (const struct file_info *)a;
    const struct file_info *file_b = (const struct file_info *)b;
    return strcmp(file_a->name, file_b->name);
}

// List directory contents in ls -l style, sorted alphabetically
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

    // Count total entries first
    uint32_t total_entries = 0;
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
        for (uint32_t j = 0; j < DIR_ENTRIES_PER_BLOCK; j++) {
            if (entries[j].inode != 0) {
                total_entries++;
            }
        }
    }

    // Allocate array for all entries
    struct file_info *files = malloc(total_entries * sizeof(struct file_info));
    if (!files) {
        fprintf(stderr, "Error allocating memory\n");
        return EXIT_FAILURE;
    }

    // Read all entries into array
    uint32_t file_index = 0;
    for (uint16_t i = 0; i < root_inode.blocks; i++) {
        int block_num = get_block_number_at(image_path, &root_inode, i);
        if (block_num <= 0) {
            free(files);
            return EXIT_FAILURE;
        }

        uint8_t data_buf[BLOCK_SIZE];
        if (read_block(image_path, block_num, data_buf) != 0) {
            free(files);
            return EXIT_FAILURE;
        }

        struct dir_entry *entries = (struct dir_entry *)data_buf;
        for (uint32_t j = 0; j < DIR_ENTRIES_PER_BLOCK; j++) {
            if (entries[j].inode != 0) {
                files[file_index].inode_num = entries[j].inode;
                strncpy(files[file_index].name, entries[j].name, FILENAME_MAX_LEN);
                
                // Read inode data
                if (read_inode(image_path, entries[j].inode, &files[file_index].inode_data) != 0) {
                    fprintf(stderr, "Error reading inode %u\n", entries[j].inode);
                    free(files);
                    return EXIT_FAILURE;
                }
                
                file_index++;
            }
        }
    }

    // Sort entries by name
    qsort(files, total_entries, sizeof(struct file_info), compare_names);

    // Print header
    printf("Inode Type Permissions Owner      Group       Blocks     Size Created             Modified            Accessed            Name\n");
    printf("===== ==== =========== ========== ========== ====== ======== =================== =================== =================== ====\n");

    // Print sorted entries
    for (uint32_t i = 0; i < total_entries; i++) {
        print_inode(&files[i].inode_data, files[i].inode_num, files[i].name);
    }

    free(files);
    return EXIT_SUCCESS;
}