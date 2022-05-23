#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#define ENTRY_SIZE 1024
#define CLUSTER_SIZE 4096

void hexToChar(unsigned char* name, int length);

void nameInfo(int fd, unsigned long long mftAddr, int entry, char *fileName){
	unsigned char buf[ENTRY_SIZE];
	unsigned char fileBuffer[CLUSTER_SIZE];
	unsigned char attName[100];
	int retVal;
	unsigned long long curLoc;
	unsigned long long offset;
	unsigned long long dataOffset;
	unsigned long long fileSize = 0;
	uint16_t firstAttOff;
	uint32_t firstAttLen;
	uint32_t fileNameLen;
	uint16_t dataRunOffset;
	uint8_t nameLength;
        uint16_t delete_flag;
        uint8_t fd_flag;
	uint8_t res_flag;
	uint8_t dataPair;
	uint32_t attLen;
	uint32_t numOfClusters;
	int32_t startCluster = 0;

	//go to entry offset
	offset = mftAddr + entry * ENTRY_SIZE;
	curLoc = lseek(fd, offset, SEEK_SET);
	if(curLoc <=0){
		fprintf(stderr, "nameInfo: unable to lseek \n");
	}
	fprintf(stdout, "Entry address: %llx\n", offset);
	//read entry
	retVal = read(fd, buf, ENTRY_SIZE);
	if(retVal <0){
		fprintf(stderr, "nameInfo: unable to read disk, retval = %d\n", retVal);
	}

	//get delete or inuse flag
	offset = 22;
	memcpy(&delete_flag, buf + offset, sizeof(delete_flag));

	//traverse to name attribute:
	//find first attribute bytes 0x14-0x15
	memcpy(&firstAttOff, buf + 20, sizeof(firstAttOff));
	//length of first attribute bytes 0x4-0x8
	offset = 4;
	memcpy(&firstAttLen, buf + firstAttOff + offset, sizeof(firstAttLen));
	//go to next attribute first attribute + length of first
	// at 0x30
	offset = firstAttOff + firstAttLen;
	//fprintf(stdout, "0x30 = %lld\n", offset);
	//get file_name length
	memcpy(&fileNameLen, buf + offset + 4, sizeof(fileNameLen));
	//fprintf(stdout, "fileNameLen (70) %d\n", fileNameLen);
	dataOffset = offset + fileNameLen;
	//fprintf(stdout, "dataOff %llx\n", dataOffset);
	//header is 24 bytes then
	offset += 24;
	//flags 0x38-0x3b: 0x38 = 56
	offset += 56;

	//get dir or file flag
	offset += 3;
	memcpy(&fd_flag, buf + offset, sizeof(fd_flag));

	//filename length offset 0x40
	//filename offset 0x41
	offset+= 5;
	memcpy(&nameLength, buf + offset, sizeof(nameLength));
	offset+= 2;
	memcpy(&attName, buf +  offset, nameLength*2);

	//print name
	fprintf(stdout, "File name: ");
	hexToChar(attName, nameLength);
	//fprintf(stdout, "length: %x name: %s", nameLength, attName);

	fprintf(stdout, "\nFile or Directory: ");
	//print file or dir
	if(fd_flag == 0x10){
		fprintf(stdout, "DIRECTORY\n");
	}
	else{
		fprintf(stdout, "FILE\n");
	}

	//print delete or inuse
	//fprintf(stdout, "Status: %x\n", delete_flag);
	if(delete_flag == 0){
		fprintf(stdout, "FILE DELETED\n");
	}
	else if(delete_flag == 1){
		fprintf(stdout, "FILE INUSE\n");
	}
	else if(delete_flag == 2){
		fprintf(stdout, "DIRECTORY DELETED\n");
	}
	else{
		fprintf(stdout, "DIRECTORY INUSE\n");
	}

	//data attribute

	//print resident flag
	//01 - non-resident : 00 - resident
	// add 8 bytes for length and attribute
	memcpy(&attLen, buf + dataOffset + 4, sizeof(attLen));
	dataOffset += attLen;
	//fprintf(stdout, "dataOffset %llx", dataOffset);
	memcpy(&res_flag, buf + dataOffset + 8, sizeof(res_flag));

	//open recovery file
        int fd2 = open(fileName, O_WRONLY);
                if(fd < 0){
                        fprintf(stderr, "Error opening recovery file.\n");
                }

	if(res_flag == 1){
		fprintf(stdout, "resident/non-resident flag: NON-RESIDENT\n");
		//print data pairs
		memcpy(&dataRunOffset, buf + dataOffset + 32, sizeof(dataRunOffset));
		//loop to go through all data pairs
		dataOffset += dataRunOffset;
		dataPair = 1;
		while(dataPair != 0){
			//read in data pair
			memcpy(&dataPair, buf + dataOffset, sizeof(dataPair));
			//fprintf(stdout, "data pair: %x\n", dataPair);
			//split data pair
			int dataOne = dataPair / 16;
			int dataTwo = dataPair % 16;

			memcpy(&startCluster, buf + dataOffset + dataTwo + 1, dataOne);
			memcpy(&numOfClusters, buf + dataOffset + 1, dataTwo);
			fileSize += numOfClusters * 1024 * 4;

			//add data pair to offset
			dataOffset += dataOne + dataTwo + 1;
			//print data pairs
			if(dataOne != 0 && dataTwo != 0){
				fprintf(stdout, "Number of clusters: %x\n", numOfClusters);
				fprintf(stdout, "Start cluster: %x\n", startCluster);
			}
			//read buffer
			startCluster *= 0x1000;
			startCluster += mftAddr;
			startCluster -= 0x4000;
			int bytesLeft = fileSize;
			int test = 1;
			while(numOfClusters > 0){
				curLoc = lseek(fd, startCluster, SEEK_SET);
				if(bytesLeft >= 4096){
					retVal = read(fd, fileBuffer, CLUSTER_SIZE);
					retVal = write(fd2, fileBuffer, CLUSTER_SIZE);
					if(retVal < 0){
						fprintf(stderr, "Error writing to file.\n");
					}
					numOfClusters -= 1;
					bytesLeft -= 4096;
					startCluster + 4096;
				}
				else{
					retVal = read(fd, fileBuffer, bytesLeft);
					write(fd2, fileBuffer, bytesLeft);
					numOfClusters -= 1;
				}
				startCluster += 4096;
			}
		}
		fprintf(stdout, "Size of file: %lld bytes\n", fileSize);
		fprintf(stdout, "Done recovering.\n");
	}
	else{
		fprintf(stdout, "resident/non-resident flag: RESIDENT\n");
		memcpy(&fileSize, buf + dataOffset + 16, 1);
		fprintf(stdout, "Size of file: %lld bytes\n", fileSize);
	}
	close(fd2);
}

void hexToChar(unsigned char*name, int length){
	for(int i = 0; i<length*2; i++){
		switch(name[i]){
			case 0x20:
				fprintf(stdout, " ");
				break;
                        case 0x21:
                                fprintf(stdout, "!");
                                break;
                        case 0x24:
                                fprintf(stdout, "$");
                                break;
                        case 0x30:
                                fprintf(stdout, "0");
                                break;
                        case 0x31:
                                fprintf(stdout, "1");
                                break;
                        case 0x32:
                                fprintf(stdout, "2");
                                break;
                        case 0x33:
                                fprintf(stdout, "3");
                                break;
                        case 0x34:
                                fprintf(stdout, "4");
                                break;
                        case 0x35:
                                fprintf(stdout, "5");
                                break;
                        case 0x36:
                                fprintf(stdout, "6");
                                break;
                        case 0x37:
                                fprintf(stdout, "7");
                                break;
                        case 0x38:
                                fprintf(stdout, "8");
                                break;
                        case 0x39:
                                fprintf(stdout, "9");
                                break;
                        case 0x41:
                                fprintf(stdout, "A");
                                break;
                        case 0x42:
                                fprintf(stdout, "B");
                                break;
                        case 0x43:
                                fprintf(stdout, "C");
                                break;
			case 0x44:
                                fprintf(stdout, "D");
                                break;
                        case 0x45:
                                fprintf(stdout, "E");
                                break;
                        case 0x46:
                                fprintf(stdout, "F");
                                break;
                        case 0x47:
                                fprintf(stdout, "G");
                                break;
                        case 0x48:
                                fprintf(stdout, "H");
                                break;
                        case 0x49:
                                fprintf(stdout, "I");
                                break;
                        case 0x4A:
                                fprintf(stdout, "J");
                                break;
                        case 0x4B:
                                fprintf(stdout, "K");
                                break;
                        case 0x4C:
                                fprintf(stdout, "L");
                                break;
                        case 0x4D:
                                fprintf(stdout, "M");
                                break;
                        case 0x4E:
                                fprintf(stdout, "N");
                                break;
                        case 0x4F:
                                fprintf(stdout, "O");
                                break;
                        case 0x50:
                                fprintf(stdout, "P");
                                break;
                        case 0x51:
                                fprintf(stdout, "Q");
                                break;
			case 0x52:
                                fprintf(stdout, "R");
                                break;
                        case 0x53:
                                fprintf(stdout, "S");
                                break;
                        case 0x54:
                                fprintf(stdout, "T");
                                break;
                        case 0x55:
                                fprintf(stdout, "U");
                                break;
                        case 0x56:
                                fprintf(stdout, "V");
                                break;
                        case 0x57:
                                fprintf(stdout, "W");
                                break;
                        case 0x58:
                                fprintf(stdout, "X");
                                break;
                        case 0x59:
                                fprintf(stdout, "Y");
                                break;
                        case 0x5A:
                                fprintf(stdout, "Z");
                                break;
                        case 0x61:
                                fprintf(stdout, "a");
                                break;
                        case 0x62:
                                fprintf(stdout, "b");
                                break;
                        case 0x63:
                                fprintf(stdout, "c");
                                break;
			case 0x64:
                                fprintf(stdout, "d");
                                break;
                        case 0x65:
                                fprintf(stdout, "e");
                                break;
                        case 0x66:
                                fprintf(stdout, "f");
                                break;
                        case 0x67:
                                fprintf(stdout, "g");
                                break;
                        case 0x68:
                                fprintf(stdout, "h");
                                break;
                        case 0x69:
                                fprintf(stdout, "i");
                                break;
                        case 0x6A:
                                fprintf(stdout, "j");
                                break;
                        case 0x6B:
                                fprintf(stdout, "k");
                                break;
                        case 0x6C:
                                fprintf(stdout, "l");
                                break;
                        case 0x6D:
                                fprintf(stdout, "m");
                                break;
                        case 0x6E:
                                fprintf(stdout, "n");
                                break;
                        case 0x6F:
                                fprintf(stdout, "o");
                                break;
                        case 0x70:
                                fprintf(stdout, "p");
                                break;
                        case 0x71:
                                fprintf(stdout, "q");
                                break;
			case 0x72:
                                fprintf(stdout, "r");
                                break;
                        case 0x73:
                                fprintf(stdout, "s");
                                break;
                        case 0x74:
                                fprintf(stdout, "t");
                                break;
                        case 0x75:
                                fprintf(stdout, "u");
                                break;
                        case 0x76:
                                fprintf(stdout, "v");
                                break;
                        case 0x77:
                                fprintf(stdout, "w");
                                break;
                        case 0x78:
                                fprintf(stdout, "x");
                                break;
                        case 0x79:
                                fprintf(stdout, "y");
                                break;
                        case 0x7A:
                                fprintf(stdout, "z");
                                break;
			case 0x2E:
				fprintf(stdout, ".");
				break;
		}
	}

}
