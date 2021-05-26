/* 
Section: 900
Group Members:
Hardik Shukla 1001664934
Utkarsh Verma 1001663173
*/
// The MIT License (MIT)
// 
// Copyright (c) 2020 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <ctype.h>


#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

struct __attribute__((__packed__)) DirectoryEntry
{
  char    DIR_Name[11]; 
  uint8_t   DIR_Attr;
  uint8_t   unused1[8];
  uint16_t  DIR_FirstClusterHigh;
  uint8_t   unused2[4];
  uint16_t  DIR_FirstClusterLow;
  uint32_t  DIR_FileSize;
};
struct DirectoryEntry dir[16];


uint16_t BPB_BytsPerSec;
uint8_t BPB_SecPerClus;
uint16_t BPB_RsvdSecCnt;
uint8_t BPB_NumFATs;
uint32_t BPB_FATSz32;

FILE *fp;

int file_open=0;
/*
 * Function     : LBAToOffset
 * Parameters   : The current sector number that points to a block of data
 * Returns:     : The value of the address for that block of data
 * Description  : Finds the starting addresss of a block of data given the sector number
 * corresponding to that data block.
*/
int LBAToOffset(int32_t sector)
{
  return ((sector-2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}
/* Function: NextLB
 * Purpose Given a logical block address, look up into the first FAT 
 * and return the logical block address of the block in the file. 
 * If there is no further blocks then return -1
*/
int16_t NextLB(uint32_t sector)
{
    uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector * 4);
    int16_t val;
    fseek(fp, FATAddress, SEEK_SET);
    fread(&val, 2, 1, fp);
    return val;
}

int compare(char *userString ,char *directoryString)
{

  const char *dotdot="..";

  if(strncmp(dotdot,userString,2)==0)
  {
    if(strncmp(dotdot,directoryString,2)==0)
    {
      return 1; // that's a match
    }
    return 0;
  }

  char IMG_Name[12];
  strncpy( IMG_Name, directoryString, 11 );
  IMG_Name[11] = '\0';

  char input[11] ;
  memset( input, 0, 11 );
  strncpy(  input, userString, strlen(userString) );

  char expanded_name[12];
  memset( expanded_name, ' ', 12 );

  char *token = strtok( input, "." );

  strncpy( expanded_name, token, strlen( token ) );

  token = strtok( NULL, "." );

  if( token )
  {
    strncpy( (char*)(expanded_name+8), token, strlen(token ) );
  }

  expanded_name[11] = '\0';

  int i;
  for( i = 0; i < 11; i++ )
  {
    expanded_name[i] = toupper( expanded_name[i] );
  }

  if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
  {
    return 1;
  }
  return 0;
}

#define MAX_NUM_ARGUMENTS 4
#define NUM_ENTRIES 16

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

int bpb()
{
  
  printf("BPB_BytsPerSec: %4d 0x%x\n",BPB_BytsPerSec,BPB_BytsPerSec);
  printf("BPB_SecPerClus: %4d 0x%x\n",BPB_SecPerClus,BPB_SecPerClus);
  printf("BPB_RsvdSecCnt: %4d 0x%x\n",BPB_RsvdSecCnt,BPB_RsvdSecCnt);
  printf("BPB_NumFATs:    %4d 0x%x\n",BPB_NumFATs ,BPB_NumFATs);
  printf("BPB_FATSz32:    %4d 0x%x\n",BPB_FATSz32,BPB_FATSz32);

  return 0;
}

int ls()
{
  int i;
  for(i=0;i<NUM_ENTRIES;i++)
  {
    
    char filename[12];
    strncpy(filename,dir[i].DIR_Name,11);
    filename[11]='\0';


    if((dir[i].DIR_Attr == ATTR_READ_ONLY||dir[i].DIR_Attr== ATTR_DIRECTORY
    ||dir[i].DIR_Attr==ATTR_ARCHIVE)&&(filename[0] != 0xffffffe5 ))
    {
      printf("%s\n",filename);
    }
    
  }
  return 0;
}

int cd(char * directoryName)
{
  // loop over the directory and search for the directory name and 
  // if the directory name is found then get the low cluster number of the directory
  // if th ecluster number is zero then set the cluster number to 2
  // use LABTOFFSEt to get the offset of the directory
  //The fseek to the offset
  //Read the directory information in the directory array 
  int i;
  int found =0;
  for(i=0; i<NUM_ENTRIES; i++)
  {
    if(compare(directoryName,dir[i].DIR_Name))
    {
      int cluster=dir[i].DIR_FirstClusterLow;
      if(cluster==0)
      {
        cluster=2;
      }
      int offset= LBAToOffset(cluster);
      fseek(fp,offset,SEEK_SET);

      fread(dir,sizeof(struct DirectoryEntry),NUM_ENTRIES,fp);
      found =1;
      break;
    }
  }
  if(!found)
  {
  printf("Error: Directory not found!\n");
  return -1;
  }

  return 0;
  
}

int statFile(char * fileName)
{
  
  int i;
  int found =0;
  for(i=0; i<NUM_ENTRIES; i++)
  {
    if(compare(fileName,dir[i].DIR_Name))
    {
      char name[12];
      strncpy(name,dir[i].DIR_Name,11);
      name[11]='\0';
      printf("%s Attr: %d Size: %d Cluster: %d\n",name,dir[i].DIR_Attr,dir[1].DIR_FileSize,dir[i].DIR_FirstClusterLow);
      found=1;
    }

  }
  if(!found)
  {
    printf("Error: File Not Found.\n");
  }

  return 0;
}

int getFile(char* originalFilename, char *newFileName)
{
  FILE *ofp;

  if (newFileName ==NULL)
  {
    ofp =fopen(originalFilename,"w");
    if(ofp==NULL)
    {
      printf("Couldn't open the File %s\n",originalFilename);
      perror("Error:  ");
    }
  }
  else
  {
    ofp=fopen(newFileName,"w");
    if(ofp==NULL)
    {
      printf("Couldn't open the File %s\n",newFileName);
      perror("Error:  ");

    }
  }
  
  int i;
  int found =0;
  for(i=0; i<NUM_ENTRIES; i++)
  {
    if(compare(originalFilename,dir[i].DIR_Name))
    {
      int cluster=dir[i].DIR_FirstClusterLow;
      found =1;

      int bytesRemainingToRead=dir[i].DIR_FileSize;
      int offset= 0;
      
      unsigned char buffer [512];
      // Handling the mid-section of the file block How is it the middle block
      // it's middle if blocks on either side are full
      while(bytesRemainingToRead>=BPB_BytsPerSec)
      {
        offset=LBAToOffset(cluster);
        fseek(fp,offset, SEEK_SET );
        fread(buffer,1, BPB_BytsPerSec,fp);

        fwrite(buffer,1,512,ofp);

        cluster =NextLB(cluster);

        bytesRemainingToRead= bytesRemainingToRead-BPB_BytsPerSec;
      }
      // Handling the last block
      if(bytesRemainingToRead)
      {

        offset=LBAToOffset(cluster);
        fseek(fp,offset, SEEK_SET );
        fread(buffer,1, bytesRemainingToRead,fp);

        fwrite(buffer,1,bytesRemainingToRead,ofp);
  
      }
      fclose(ofp);
    }
  }
  return 0;
}


int readFile(char * fileName, int requestedOffset, int requestedBytes)
{
  int i;
  int found =0;
  int bytesRemainingToRead = requestedBytes;
  if(requestedOffset<0)
  {
    printf("Error: Offset can't be less than zero \n");
  }
  //for the command read num.txt 510 4
  //requested_offset =510
  // requested_bytes =4;
  for(i=0; i<NUM_ENTRIES; i++)
  {
    if(compare(fileName,dir[i].DIR_Name))
    {
      int cluster=dir[i].DIR_FirstClusterLow;
      found =1;
      //dummy loop to print out all the clusters of the file
      int searchSize= requestedOffset;
      // Searching the beginning cluster of the file
      // To read num.txt 512, We need to begin with the second cluster
      while(searchSize>= BPB_BytsPerSec)
      {
        
        cluster=NextLB(cluster);
        searchSize=searchSize-BPB_BytsPerSec;

      }
      //printf("Your Cluster is %d\n",cluster);
      // Reading the first block
      int offset= LBAToOffset(cluster);
      int byteOffset=( requestedOffset % BPB_BytsPerSec);
      fseek(fp,offset + byteOffset, SEEK_SET );
      
      unsigned char buffer [BPB_BytsPerSec];
      // figure out the number of bytes to be read in the first block
      int firstBlockBytes = BPB_BytsPerSec-requestedOffset;
      fread(buffer,1, firstBlockBytes,fp);
      for(i=0;i<firstBlockBytes;i++)
      {
        printf("%x ",buffer[i]);
      }
      
      bytesRemainingToRead= bytesRemainingToRead-firstBlockBytes;
      // Handling the mid-section of the file block How is it the middle block
      // it's middle if blocks on either side are full
      while(bytesRemainingToRead>=512)
      {
        cluster= NextLB(cluster);
        offset=LBAToOffset(cluster);
        fseek(fp,offset, SEEK_SET );
        fread(buffer,1, BPB_BytsPerSec,fp);
        for(i=0;i<BPB_BytsPerSec;i++)
        {
          printf("%x ",buffer[i]);
        }
        bytesRemainingToRead= bytesRemainingToRead-BPB_BytsPerSec;
      }
      // Handling the last block
      if(bytesRemainingToRead)
      {
        cluster= NextLB(cluster);
        offset=LBAToOffset(cluster);
        fseek(fp,offset, SEEK_SET );
        fread(buffer,1, bytesRemainingToRead,fp);

        for(i=0;i<bytesRemainingToRead;i++)
        {
          printf("%x ",buffer[i]);
        }
        
      }
      printf("\n");
    }
  }
  if(!found)
  {
  printf("Error: Directory is not found!\n");
  return -1;
  }
  return 0;
}

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {

    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your FAT32 functionality

    if(strcmp(token[0],"open")==0)
    {
      if(file_open )
      {
        printf("Error: File already open!\n");
        continue;
      }
      else
      {
        fp= fopen(token[1],"r");
        if(fp==NULL)
        {
          perror("Error: File does not exist ");
          continue;
        }
      }

      if(fp==NULL)
      {
        perror("Couldn't open the file");
        printf(" %s\n",token[1]);
      }
      else
      {
        file_open=1;
        //printf("Error: File already open!");
      }
    
      //read the BPB section
      fseek(fp, 11, SEEK_SET);
      fread(&BPB_BytsPerSec,1,2,fp);

      fseek(fp,13,SEEK_SET);
      fread(&BPB_SecPerClus,1,1,fp);

      fseek(fp,14,SEEK_SET);  
      fread(&BPB_RsvdSecCnt,1,2,fp);
      
      fseek(fp,16,SEEK_SET);
      fread(&BPB_NumFATs ,1,2,fp);

      fseek(fp,36,SEEK_SET);
      fread(&BPB_FATSz32,1,4,fp);
    // the root directory address is located past the reserved sector area and both the FATs
      int rootAddress=(BPB_RsvdSecCnt*BPB_BytsPerSec)+(BPB_NumFATs*BPB_FATSz32* BPB_BytsPerSec);
      //printf("%x\n",rootAddress);
      fseek(fp,rootAddress,SEEK_SET);
      fread(dir,sizeof(struct DirectoryEntry),NUM_ENTRIES,fp);
    }
    else if(strcmp(token[0],"close")==0)
    {
      if(file_open)
      {
        fclose(fp);
        //reset the file state
        fp=NULL;
        file_open=0;
      }
      else
      {
       perror("Error :File not open\n");
      }
    }
    else if(strcmp(token[0],"bpb")==0)
    {
      if(file_open)
      {
        bpb();

      }
      else
      {
        perror("Error :File image not opened\n");
      }
      

    }
    else if(strcmp(token[0],"ls")==0)
    {
      if(file_open)
      {
        ls();
      }
     else
      {
        perror("Error :File image is not open\n");
      }
    }
    else if(strcmp(token[0],"cd")==0)
    {
      if(file_open)
      {
        cd(token[1]);
      }
     else
      {
        perror("Error :File Image not opened\n");
      }
    }
    else if(strcmp(token[0],"read")==0)
    {
      if(file_open)
      {
        readFile(token[1],atoi(token[2]),atoi(token[3]));
      }
     else
      {
        perror("Error :File image is not open\n");
      }
    }
     else if(strcmp(token[0],"get")==0)
    {
      if(file_open) 
      {
        getFile(token[1],token[2]);
      }
     else
      {
        perror("Error :File image is not open\n");
      }
    }
    else if(strcmp(token[0],"stat")==0)
    {
      if(file_open) 
      {
        statFile(token[1]);
      }
     else
      {
        perror("Error :File image is not open\n");
      }
    }
    

    free( working_root );

  }

  return 0;
}
