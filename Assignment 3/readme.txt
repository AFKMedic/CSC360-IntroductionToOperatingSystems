*********
Diskinfo*
*********

Executed using:
./diskinfo image
Where image is the image files you want to use.

Reads in the superblock information and stores the information into a struct.
Using that information the blocks for the FAT entries are found and iterated
over entry by entry.
The free, reserved, and allocated blocks are then tallied up according to 
the contents of the entry.
The information is then printed. 

*********
Disklist*
*********

Executed using:
./disklist image
Where image is the image files you want to use.

Reads in the superblock information and stores the information into a struct.
Using that information the blocks for the root directory are found and iterated
over entry by entry.
Iterates over the root folder and stores information about an entry into a struct.
It is assumed the entries are all 64 bytes long.
If the entry is not empty then the information is printed.

*********
Diskget *
*********

Executed using:
./diskget image foo bar
Where image is the image files you want to use, foo is the file from the 
image and bar is the output file.

Reads in the superblock information and stores the information into a struct.
Using that information the blocks for the root directory are found and iterated
over entry by entry.
Finds correct directory entry by finding the matching filename.
Finds FAT entry and starting block from directory entry.
Finds File data using the FAT entries and writes the information to a buffer.
Information from the buffer is written to the output file.

*********
Diskput *
*********

Executed using:
./diskput image foo bar
Where image is the image files you want to use, foo is the file to be read from
and bar is the file wirtten to the image.

Not writting properly.
Firsts checks if the file already exists or if there is enough space. If file exists
or there is not enough space then print error and exit.
Iterates through the FAT entries to find enough free blocks to store the file.
Update the FAT entries and write the data into repective blocks.
Update entry within the root folder with information about the new file.