/*
P3 Part 3
Chris Wong
V00780634
*/

#include <ctype.h> 
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

// Converts a string to uppercase
void stringUpper(char* s){
    char *pnt= s;
    while (*pnt) {
        *pnt = toupper((unsigned char) *pnt);
        pnt++;
    }
}

// Writes all data from a buffer into a file
void writeToFile(FILE *f, char *name, u_int8_t buffer[], int size){
    f = fopen(name,"w");

    for(int i = 0; i < size; i++){
        fprintf(f,"%c",buffer[i]);
    }

    fclose(f);
}

void main(int argc, char *argv[]){
    int fileRead = open(argv[1], O_RDWR);
    int foundFile = 0;
    char* fileName = argv[2];
    char* outputName = argv[3];
    FILE *fileWrite;

    struct stat buffer;
    int status = fstat(fileRead, &buffer);

    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileRead, 0);

    struct superblock_t* sb;
    sb = (struct superblock_t*)address;

    // Find address of start and end of root folder
    void* rootPointer = address+((int) htons(sb->block_size) * (int)ntohl(sb->root_dir_start_block));
    void* rootEnd = rootPointer + ((int) htons(sb->block_size) * ntohl(sb->root_dir_block_count));
    struct dir_entry_t* dir;

    struct dir_entry_time_t createDate;

    while(rootPointer != rootEnd){
        dir = (struct dir_entry_t*)rootPointer;
        createDate = (struct dir_entry_time_t) dir->create_time;

        // Get copy of filename as to not edit memory
        char *tmp = strdup((char *) dir->filename);

        // Convert to uppercase for comparison
        stringUpper(fileName);
        stringUpper(tmp);

        if(!strcmp(tmp, fileName)){
            foundFile = 1;
            break;
        }

        rootPointer = rootPointer + 64;
    }

    // Exit if file is not found
    if(foundFile == 0){
        printf("File not found.\n");
        exit(0);
    }
    
    int writeCounter = 0;
    u_int8_t writeBuffer[ntohl(dir->size)];

    // Set up pointers
    void* FATPointer =  address + ((int) htons(sb->block_size) * (int)ntohl(sb->fat_start_block)) + (ntohl(4 * dir->starting_block));
    void* dataPointer = address + ((int) htons(sb->block_size) * (int)ntohl(dir->starting_block));
    int nextBlock = (int)ntohl(dir->starting_block);

    do{
        // copy data
        for(int i = 0; i < (int) htons(sb->block_size); i++){
            if(writeCounter < dir->size) memcpy(&writeBuffer[writeCounter], dataPointer, 4);
            dataPointer++;
            writeCounter++;
        }    

        // Update pointer
        FATPointer =  address + ((int) htons(sb->block_size) * (int)ntohl(sb->fat_start_block)) + (4 * nextBlock);
        memcpy(&nextBlock, FATPointer, 4);
        nextBlock = ntohl(nextBlock);
        void* dataPointer = address + ((int) htons(sb->block_size) * nextBlock);

    }while(nextBlock != -1);

    writeToFile(fileWrite, outputName, writeBuffer, ntohl(dir->size));

    munmap(address,buffer.st_size);
    close(fileRead);
}