/*
P3 Part 1
Chris Wong
V00780634
*/
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct __attribute__((__packed__)) superblock_t {
    uint8_t     fs_id [8];
    uint16_t    block_size;
    uint32_t    file_system_block_count;
    uint32_t    fat_start_block;
    uint32_t    fat_block_count;
    uint32_t    root_dir_start_block;
    uint32_t    root_dir_block_count;
};

void main(int argc, char* argv[]) {
    int file = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(file, &buffer);

    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);

    struct superblock_t* sb;
    sb = (struct superblock_t*)address;

    // Iterate through FAT blocks and count
    void* FATPointer =  address+((int) htons(sb->block_size) * (int)ntohl(sb->fat_start_block));
    void* FATEnd = FATPointer + ((int) htons(sb->block_size) * ntohl(sb->fat_block_count));
    int totalBlocks = ntohl(sb->fat_block_count) - ntohl(sb->fat_start_block);
    int fssize;
    int freeBlocks = 0;
    int reservedBlocks = 0;
    int allocatedBlocks = 0;

    while(FATPointer != FATEnd){
        memcpy(&fssize, FATPointer, 4);
        fssize=ntohl(fssize);
        if(fssize == 0x00){
            freeBlocks++;
        }else if(fssize == 0x01){
            reservedBlocks++;
        }else{
            allocatedBlocks++;
        }

        FATPointer = FATPointer + 4;
    }

    // Print information
    printf("Block size: %d\n", htons(sb->block_size));
    printf("Block count: %d\n", ntohl(sb->file_system_block_count));
    printf("FAT starts: %d\n", ntohl(sb->fat_start_block));
    printf("FAT blocks: %d\n", ntohl(sb->fat_block_count));
    printf("Root directory start: %d\n", ntohl(sb->root_dir_start_block));
    printf("Root directory blocks: %d\n\n", ntohl(sb->root_dir_block_count));
    printf("FAT information:\n");
    printf("FAT information:\n");
    printf("Free blocks: %d\n", freeBlocks);
    printf("Reserverd blocks: %d\n", reservedBlocks);
    printf("Allocated blocks: %d\n", allocatedBlocks);

    munmap(address,buffer.st_size);
    close(file);
}
