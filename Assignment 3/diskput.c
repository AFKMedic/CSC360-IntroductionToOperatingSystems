/*
P3 Part 4
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
#include <time.h>

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

void stringUpper(char* s){
    char *pnt= s;
    while (*pnt) {
        *pnt = toupper((unsigned char) *pnt);
        pnt++;
    }
}

void main(int argc, char *argv[]){
    int fileWrite = open(argv[1], O_RDWR);
    int fileRead;
    
    //Check if file exists to read
    if( access(argv[2], F_OK) == 0 ) {
         fileRead = open(argv[2], O_RDWR);
    } else {
        printf("File not found.\n");
        exit(0);
    }

    char* outputName = argv[3];
    struct superblock_t* sb;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    //Set up buffer for destination
    struct stat buffer;
    int status = fstat(fileWrite, &buffer);
    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileWrite, 0);

    // Set up buffer for file
    struct stat readBuffer;
    int readStatus = fstat(fileRead, &readBuffer);
    void* filePntr = mmap(NULL, readBuffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileRead, 0);

    sb = (struct superblock_t*)address;
    struct dir_entry_t* dir;
    struct dir_entry_time_t createDate;
    const int blockSize = (int) htons(sb->block_size);
    
    // Get pointers for start and end of FAT Table
    void* FATPointer =  address+(blockSize * (int)ntohl(sb->fat_start_block));
    void* FATEnd = FATPointer + (blockSize * ntohl(sb->fat_block_count));

    // Get pointers for start and end of root folder
    void* rootPointer = address+(blockSize * (int)ntohl(sb->root_dir_start_block));
    void* rootEnd = rootPointer + (blockSize * ntohl(sb->root_dir_block_count));
    
    //Check if file already exists in the root directory
    while(rootPointer != rootEnd){
        dir = (struct dir_entry_t*)rootPointer;
        createDate = (struct dir_entry_time_t) dir->create_time;

        // Get copy of filename as to not edit memory
        char *tmp = strdup((char *) dir->filename);

        // Convert to uppercase for comparison
        stringUpper(argv[3]);
        stringUpper(tmp);

        if(!strcmp(tmp, argv[3])){
            printf("File already exists.\n");
            exit(0);
        }
        rootPointer = rootPointer + 64;
    }

    // Check space requirements
    int fssize;
    int freeBlocks = 0;
    while(FATPointer != FATEnd){
        memcpy(&fssize, FATPointer, 4);
        fssize=ntohl(fssize);
        if(fssize == 0x00) freeBlocks++;
        FATPointer = FATPointer + 4;
    }

    if(freeBlocks * htons(sb->block_size) < readBuffer.st_size){
        printf("Not enough space avaliable.\n");
        exit(0);
    }
    
    //Reset to respective start blocks
    FATPointer = address+(blockSize * (int)ntohl(sb->fat_start_block));
    rootPointer = address+(blockSize * (int)ntohl(sb->root_dir_start_block));

    // Get free entries in FAT that will be used for writing
    int FATEntry = 0;
    int usedBlocks[(readBuffer.st_size + blockSize - 1)/ blockSize];
    int counter = 0;
    while(counter != (readBuffer.st_size + blockSize - 1)/ blockSize){
        memcpy(&fssize, FATPointer, 4);
        fssize=ntohl(fssize);
        void* tmpPntr;
        int tmpEntry = ntohl(FATEntry);
        if(fssize == 0x00){
            usedBlocks[counter] = FATEntry;
            counter++;

        }
        FATEntry++;
        FATPointer = FATPointer + 4;
    }

    //Write Entries into FAT
    for(int i = 0; i < (readBuffer.st_size + blockSize - 1)/ blockSize; i++){
        if(i == 0){
            FATPointer = address+(blockSize * usedBlocks[i]);
            int tmp = usedBlocks[1];
            memcpy(&FATPointer, &tmp, 4);
        }else if(i = (readBuffer.st_size + blockSize - 1)/ blockSize - 1) {
            FATPointer = address+(blockSize * usedBlocks[i]);
            u_int32_t max = 0xFFFFFFFF;
            memcpy(&FATPointer, &max, 4);
            memcpy(&address, &max, 4);
        }else{
            FATPointer = address+(blockSize * usedBlocks[i]);
            int tmp = usedBlocks[i + 1];
            memcpy(&FATPointer, &tmp, 4);
        }
    }

    void* writePointer;
    int remainingBytes = readBuffer.st_size;
    // Write data
    for(int i = 0; i < (readBuffer.st_size + blockSize - 1)/ blockSize; i++){
         writePointer = address + (blockSize * usedBlocks[i]);
        for(int j = 0; j < blockSize; j++){
            memcpy(&writePointer, &filePntr, 1);
            filePntr++;
            writePointer++;
        }
    }
    

    // Get first free spot in directory
    while(rootPointer != rootEnd){
        dir = (struct dir_entry_t*)rootPointer;
        createDate = (struct dir_entry_time_t) dir->create_time;
        if(dir->status == 0x00) break;
        rootPointer = rootPointer + 64;
    }
   
    dir->status = 0x01;
    dir->starting_block = ntohl(usedBlocks[0] * blockSize);
    dir->block_count = ntohl((readBuffer.st_size + blockSize - 1)/ blockSize);
    createDate.year = htons(tm.tm_year);
    createDate.month = tm.tm_mon;
    createDate.day = tm.tm_mday;
    createDate.hour = tm.tm_hour;
    createDate.minute = tm.tm_min;
    createDate.second = tm.tm_sec;
    dir->create_time = (struct dir_entry_time_t) createDate;
    dir->modify_time = (struct dir_entry_time_t) createDate;
    dir->unused[5] = 0xF;
    dir->unused[6] = 0xF;
    
    munmap(address,buffer.st_size);
    munmap(filePntr,readBuffer.st_size);
    close(fileRead);
    close(fileWrite);
}