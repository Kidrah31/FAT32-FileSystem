# FAT32-FileSystem
A FAT-32 a user space shell application that is capable of interpreting a FAT-32 formatted disk image, and allows the user to traverse the file system  as well as copy files out or read the files that are contained in the file system.
***
## Requirements
  FAT-32 fromatted diak image
  ***
## Build and Run
 To build the project use a Vm or Linux/Unix terminal, enter:
 
    make
 
 To run the project enter:
 
    ./mfs.e
 ***
## Accepted commands
  The program prompts `mfs>` when it is ready to accept input and accepts the following commands:
 
 ### 1. open
 
        open <filename>
    
  The `open` command shall open a FAT32 disk image.
  
 ### 2. close
  
    close   

 The `close` command closes the currently open FAT32 disk image.
    
 ### 3. bpb
  
    bpb
 
  The `bpb` command  prints out information about the file system in both hexadecimal and base 10:
   * BPB_BytesPerSec
   * BPB_SecPerClus
   * BPB_RsvdSecCnt
   * BPB_NumFATS
   * BPB_FATSz32
      
  ### 4. stat
  
    stat <filename> or <directory name>
    
   The `stat` command  prints the attributes and starting cluster number of the file or directory name specified.
   
  ### 5. read
  
    read <filename> <position> <number of bytes>
    
   The `read` command reads from the given file at the position, in bytes, specified by the position parameter and output the number of bytes specified in hexadecimal.
  
  ### 6. get
  
    get <filename> <position> <number of bytes>
    
   This `get` command  retrieves the file from the FAT 32 image and place it in the current working directory.
  
    get <filename> <new filename>
    
   This `get` command retrieves the file from the FAT 32 image and places it in the current working directory with the ne file name specified.
  
  ### 7. cd
  
    cd <directory>
    
   The `cd` command changes the current working directory to the given directory and also supports relative paths.
  
  ### 8.  ls
  
    ls
  
   The `ls` command lists the directory contents
  
 ***
## Test Run Demo
 ![Demo-run](https://github.com/Kidrah31/FAT32-FileSystem/blob/main/FileSystem_demo.gif)
 ***
 ## Resources
   [Course-Github-Repo](https://github.com/CSE3320/FAT32-Assignment)

***
