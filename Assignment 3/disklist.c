/*
P3 Part 2
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

struct __attribute__((__packed__))superblock_t{ 
    uint8_t     fs_id [8];
    uint16_t    block_size;
    uint32_t    file_system_block_count; 
    uint32_t    fat_start_block;
    uint32_t    fat_block_count; 
    uint32_t    root_dir_start_block; 
    uint32_t    root_dir_block_count;
};

struct __attribute__((__packed__))dir_entry_time_t{
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     minute;
    uint8_t     second;
};

struct __attribute__((__packed__))dir_entry_t{
    uint8_t                 status;
    uint32_t                starting_block;
    uint32_t                block_count;
    uint32_t                size;
    struct dir_entry_time_t create_time;
    struct dir_entry_time_t modify_time;  
    uint8_t                 filename[31];
    uint8_t                 unused[6];
};

void print(struct dir_entry_t* dir, struct dir_entry_time_t createDate){
    
    if(dir->status == 0x06 || dir->status ==  0x03){
        printf("F ");
    }else{
        printf("D ");
    }
    printf("%10d ", ntohl(dir->size));
    printf("%30s ", (char*) dir->filename);
    printf("%d/%2d/%2d ", htons(createDate.year), createDate.month,createDate.day);
    printf("%02d:%02d:%02d\n", createDate.hour, createDate.minute, createDate.second);

}

void main(int argc, char *argv[]){
    int file = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(file, &buffer);

    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);

    struct superblock_t* sb;
    sb = (struct superblock_t*)address;

    // Get pointers for start and end of root folder
    void* rootPointer = address+((int) htons(sb->block_size) * (int)ntohl(sb->root_dir_start_block));
    void* rootEnd = rootPointer + ((int) htons(sb->block_size) * ntohl(sb->root_dir_block_count));
    struct dir_entry_t* dir;

    // Go through all the blocks of the directory
    // Assumption that files or folders are in 64 byte chunks
    while(rootPointer != rootEnd){
        dir = (struct dir_entry_t*)rootPointer;
        struct dir_entry_time_t createDate = (struct dir_entry_time_t) dir->create_time;
        if(dir->status != 0x00 && dir->status != 0x04) print(dir, createDate);

        rootPointer = rootPointer + 64;
    }

    munmap(address,buffer.st_size);
    close(file);
}