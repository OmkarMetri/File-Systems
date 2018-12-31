# File-Systems
Persistent File System for UNIX based systems

## Requirements
FUSE - File System in User Space

## Installation commands
$ sudo apt-get update && sudo apt-get install libfuse-dev

$ sudo apt install pkg-config

$ sudo apt-get install pkgconf

## Execution Commands
$ gcc -Wall -o FS fsMain.c `pkg-config fuse --cflags --libs`

$ ./FS -f /path_to_Fuse_File

$ sudo umount /path_to/Fuse_Files

#### Note: Path_to_Fuse_File is an empty directory and should be created manually.
