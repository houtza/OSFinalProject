/***********************************************************
 * Name of program: fat32_reader.c
 * Authors: Aaron Houtz and Geoffrey Miller
 * Description: A basic program to interperate and display information from a FAT32 file system image.
 **********************************************************/

/* These are the included libraries.  You may need to add more. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

/* Put any symbolic constants (defines) here */
#define True 1  /* C has no booleans! */
#define False 0

#define MAX_CMD 80

uint16_t BPB_BytesPerSec=0;
uint8_t BPB_SecPerClus=0;
uint16_t BPB_RsvdsSecCnt=0;
uint8_t BPB_NumFATs=0;
uint16_t BPB_RootEntCnt=0;
uint32_t BPB_FATSz32=0;
uint32_t BPB_RootClus=0;
uint32_t rootDirAddress=0; 
uint32_t currentCluster=0;

uint32_t RootDirSectors();//Find the root directory sector
uint32_t FirstDataSectors();//Find first data sectorsudo apt-get update
uint32_t FirstSectorOfCluster();//Find the first sector of cluster
void CalcRootDir();//Calculates the starting value of the root directory
uint16_t GetBPBInfo(int fd, int byteOffset, int byteSize);//Used to access the BPB sector of memory and return the contents at the specified offset.
                                                             //Takes in the file handle, byte offset and how many bytes you want to read (1 or 2) and returns the number.
uint32_t ThisFATSecNum(uint32_t N);
uint32_t ThisFATEntOffset(uint32_t N);

/* This is the main function of your project, and it will be run
 * first before all other functions.
 */
int main(int argc, char *argv[])
{
       char cmd_line[MAX_CMD];


	uint16_t read_num = 0; /*little-endian num read from fat file*/
	uint16_t write_num = 0; /*little-endian num written to fat file*/
	uint16_t convert = 0; /*holds conversion to host architecture*/
	int fd; /*file descriptor*/
	int result; /*error checking*/
        int littleEndian=1;

	/* What architecture are we? */
	if(__BYTE_ORDER == __LITTLE_ENDIAN) {
	  //printf("Host architecutre is little-endian.\n");
	  littleEndian=1;
	} else {
	  littleEndian=0;
	  //printf("Host architecture is big-endian.\n");
	}

	/* Open a file with read/write permissions. */
	fd = open(argv[1],O_RDWR);

	/* Error checking for write. I/O functions tend to return
	   -1 if something is wrong. */
	if(fd == -1) {
	  perror(argv[1]);
	  return -1;
	}

	/* Parse args and open our image file */

	/* Parse boot sector and get information */

	BPB_BytesPerSec=GetBPBInfo(fd,11,2);

	BPB_SecPerClus=GetBPBInfo(fd,13,1);

	BPB_NumFATs=GetBPBInfo(fd,16,1);

	BPB_RootEntCnt=GetBPBInfo(fd,17,2);

	BPB_RsvdsSecCnt=GetBPBInfo(fd,14,2);

	BPB_RootClus=GetBPBInfo(fd,44,2);
	currentCluster=BPB_RootClus;

	//GetBPBInfo can return 2 bytes at most, so we must use bit shifting to get and store 4 bytes if we want to use the funtion..
	if(littleEndian==1){
	  BPB_FATSz32 = (GetBPBInfo(fd,38,2) <<16) | GetBPBInfo(fd,36,2);
	  BPB_RootClus = (GetBPBInfo(fd,46,2) <<16) | GetBPBInfo(fd,44,2);
	
	}
	else{
	  BPB_FATSz32 = (GetBPBInfo(fd,36,2) <<16) | GetBPBInfo(fd,38,2);
	  BPB_RootClus  = (GetBPBInfo(fd,44,2) <<16) | GetBPBInfo(fd,46,2);
	}

    CalcRootDir();  //Calculate the root directory address.

	/* Main loop.  You probably want to create a helper function
       for each command besides quit. */

	while(True) {
		bzero(cmd_line, MAX_CMD);
		printf("/]");
		fgets(cmd_line,MAX_CMD,stdin);

		/* Start comparing input */
		if(strncmp(cmd_line,"info",4)==0) {

		  //Print out the information.
		  printf("BPB_BytesPerSec is 0x%x, decimal: %i\n", BPB_BytesPerSec, BPB_BytesPerSec);
		  
		  printf("BPB_SecPerClus is 0x%x, decimal: %i\n", BPB_SecPerClus, BPB_SecPerClus);
		  
		  printf("BPB_RsvdsSecCnt is 0x%x, decimal: %i\n", BPB_RsvdsSecCnt, BPB_RsvdsSecCnt);

		  printf("BPB_NumFATs is 0x%x, decimal: %i\n", BPB_NumFATs, BPB_NumFATs);
		 
	      printf("BPB_FATSz32 is 0x%x, decimal: %i\n", BPB_FATSz32, BPB_FATSz32);
		  
          printf("The root address is 0x%x, decimal: %i\n",  rootDirAddress,rootDirAddress);
                  

		  //printf("BPB_RootClus is 0x%x, decimal: %i\n", BPB_RootClus, BPB_RootClus);

		  FirstSectorOfCluster(94);
		  printf("The FAT address is 0x%x",  ((ThisFATSecNum(94)*512)+ThisFATEntOffset(94)));
		}

		else if(strncmp(cmd_line,"volume",6)==0) {
			printf("Going to run volume!\n");
            int volumeCounter=0;
            uint8_t volumeName=0;
			while(volumeCounter!=11){
				lseek(fd, rootDirAddress+volumeCounter, SEEK_SET);
				read(fd,&volumeName,1);

 				volumeCounter++;
 				printf("%c\n",volumeName );
			}

			//uint64_t volumeName=0;

			//lseek(fd, rootDirAddress, SEEK_SET);
		    //read(fd,&volumeName,11);
		    //volumeName<<32;
		    //lseek(fd, rootDirAddress+4, SEEK_SET);
		    //read(fd,&volumeName,4);

		    //printf("The volume address is 0x%x", volumeName);
		    //printf("%x",volumeName );
		}
		
		else if(strncmp(cmd_line,"stat",4)==0) {
			printf("Going to stat!\n");
		}

		else if(strncmp(cmd_line,"cd",2)==0) {
			//char directoryName

			char *dirName;
			dirName=cmd_line+3;
			printf("%s\n",dirName);

			printf("Going to cd!\n");
		}

		else if(strncmp(cmd_line,"ls",2)==0) {
			uint8_t statByte=1;
			uint8_t stopByte=1;
			uint8_t lsName=0;
			int charCounter=0;
			uint32_t dirAddress=0;

			uint32_t nextCluster=currentCluster;
			uint32_t nextClusterFATaddress=0;

			
			while(nextCluster<0xF8FF0F){


				dirAddress= FirstSectorOfCluster(nextCluster)*512;

				lseek(fd, dirAddress, SEEK_SET);
				read(fd,&stopByte,1);
				while(stopByte!=0){
					lseek(fd, dirAddress+11, SEEK_SET);
					read(fd,&statByte,1);
					if(statByte==0x08){
						dirAddress=dirAddress+32;
					}


					else if(statByte==0x0F){
						dirAddress=dirAddress+32;
					}

					else if(statByte==0x10){
						while(charCounter!=11){
							
							lseek(fd, dirAddress+charCounter, SEEK_SET);
							read(fd,&lsName,1);

	 						charCounter++;
	 						if(lsName!=0x20){
	 							printf("%c",lsName);
	 						}
	 						
						}
						printf("\n");
						charCounter=0;
						dirAddress=dirAddress+32;
					}

					else if(statByte==0x20){
						while(charCounter!=11){
							if(charCounter==8){
								printf(".");
							}
							lseek(fd, dirAddress+charCounter, SEEK_SET);
							read(fd,&lsName,1);

	 						charCounter++;
	 						if(lsName!=0x20){
	 							printf("%c",lsName);
	 						}
	 						
						}
						printf("\n");
						charCounter=0;
						dirAddress=dirAddress+32;
					}




					lseek(fd, dirAddress, SEEK_SET);
					read(fd,&stopByte,1);
				}
				//uint16_t volumeName;
				//PrintLs(fd,rootDirAddress);

				//lseek(fd, startAddress, SEEK_SET);
			    //read(fd,&volumeName,sizeof(enteryType));
			    nextClusterFATaddress=((ThisFATSecNum(2)*512)+ThisFATEntOffset(2));
			    lseek(fd, nextClusterFATaddress, SEEK_SET);
				read(fd,&nextCluster,4);

			    //nextCluster=((ThisFATSecNum(2)*512)+ThisFATEntOffset(2));
			}



			printf("Going to ls.\n");
		}

		else if(strncmp(cmd_line,"read",4)==0) {
			printf("Going to read!\n");
		}
		
		else if(strncmp(cmd_line,"quit",4)==0) {
			printf("Quitting.\n");
			break;
		}
		else
			printf("Unrecognized command.\n");


	}

	/* Close the file */
	close(fd);

	return 0; /* Success */
}

//Used to access the BPB sector of memory and return the contents at the specified offset.
//Takes in the file handle, byte offset and how many bytes you want to read (1 or 2) and returns the number.
uint16_t GetBPBInfo(int fd, int byteOffset,int byteSize){

  uint16_t read_num =0;
  uint16_t convert = 0;
  int result;

  
  result = lseek(fd, byteOffset, SEEK_SET);

  /* Error checking for lseek */
  if(result == -1){
    perror("lseek");
    close(fd);
    return -1;
  }

  if(byteSize==2){
    byteSize=1;
  }else{
    byteSize=2;
  }
  
  /* Read a number from the file. This number is in little-endian. */
  result = read(fd,&read_num,(sizeof(read_num)/byteSize));

  /* Error checking for read */
  if(result==-1) {
    perror("read");
    close(fd);
    return -1;
  }


  /* Let's turn it into the host order (correct) number. This could
   * be in little-endian or big-endian format, depending on our
   * architecture.  The nice thing is that we don't have to care
   * when we write our code - the functions already "know" our
   * host order. */
  convert =  le16toh(read_num);
  //printf("That number is actually ");
  //printf("BPB_BytesPerSec is 0x%x, decimal: %i\n", convert, convert);

  return convert;
}


//Calculates the root directory sector.
uint32_t RootDirSectors(){
  uint32_t rootDirSectors;
  rootDirSectors = ((BPB_RootEntCnt * 32) + ( BPB_BytesPerSec -1)) /  BPB_BytesPerSec;
  return rootDirSectors;
}

//Calculates the first data sector.
uint32_t FirstDataSector(){
  uint32_t firstDataSector = BPB_RsvdsSecCnt + (BPB_NumFATs *  BPB_FATSz32)+ RootDirSectors();
  return firstDataSector;

}

//Calculates the root address by first calculating the first sector of cluster and then multpliess it by 512. 
void CalcRootDir(){
  rootDirAddress=512*((( BPB_RootClus-2)* BPB_SecPerClus) +FirstDataSector());
}


//Calculates the first sector of cluster and then multpliess it by 512 to print out the starting address of the root directory. 
uint32_t FirstSectorOfCluster(uint32_t n){
  uint32_t firstSectorOfCluster=(( n-2)* BPB_SecPerClus) +FirstDataSector();
  printf("The address is 0x%x, decimal: %i\n",  firstSectorOfCluster*512,  firstSectorOfCluster*512);
  return firstSectorOfCluster;
}


uint32_t ThisFATSecNum(uint32_t N){
	return (BPB_RsvdsSecCnt + ((N*4)/BPB_BytesPerSec));
}

uint32_t ThisFATEntOffset(uint32_t N){
	return ((N*4)% BPB_BytesPerSec);
}

