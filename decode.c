#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "zerg2Structs1.h"

#define NTOH2(x) (((x << 8) & 65280) + (x >> 8))
#define NTOH3(x) ((int) x[0] << 16) | ((int) (x[1]) << 8) | ((int) (x[2]))
#define NTOH4(x) ((int) x[0] << 24) | ((int) (x[1]) << 16) | ((int) (x[2]) <<  8) | ((int) (x[3]))

char * zergBreed(int breedType);
long unsigned swap32(long unsigned val);
char * commandOption(int instruction);
uint64_t ntoh64(uint8_t number[8]);
int fileSize(FILE *fp);
int hexToDec(int x);
double convert64ToDouble(uint64_t num);
double convert32ToDouble(uint32_t num);




void main(int argc, char *argv[])
{
	
	TOPHEADER fileHeader;
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL){
		printf("file does not exist\n");
		exit(1);
	}
	// CHECK file size before while loop, function automatically rewinds file
	int fullSize = fileSize(fp); 
	//int *currentSize = ftell(fp); 	
	//printf("This is file size %d\n", fullSize);


//Start while loop here
//Indent all the information below this line for while loop
	//int packetLength = hexToInt(packet.legnthDataCaptured);
	//printf("this is packet Length %d\n", packetLength);
	//while(ftell(fp) < fullSize){
		
	

	fread(&fileHeader, sizeof(fileHeader), 1, fp);
	int count = 0;
	int endLength = 0;
	
	while(ftell(fp) < fullSize)
	{

		count++;
		//printf("Packet number %d\n", count);
		PACKETHEADER packet;
		fread(&packet, sizeof(packet), 1, fp);
		int packetLength;
		//packetLength = hexToDec(packet.lengthDataCaptured);
		//printf("this is packet Length %d\n", packetLength);
	//	printf("This is packet length of data %d\n", packet.lengthDataCaptured);
	//	printf("This is the old fp pointer %d\n", ftell(fp));		
		
		packetLength = (int) packet.lengthDataCaptured;
		fseek(fp, packetLength, SEEK_CUR);
		endLength = ftell(fp);
		//printf("This is end of packet %d\n", endLength);
		//printf("This is file pointer at next packet at next pcap packet header %ld\n", ftell(fp));
		fseek(fp, -packetLength, SEEK_CUR);
		//printf("This is the new fp %ld\n", ftell(fp));
		//int y = 50;
		//if (y < packet.lengthDataCaptured){
		//	printf("y --> %d is less than length of data captured %d \n", y, packet.lengthDataCaptured);
		//}

	
		ETHERNET ether;
		fread(&ether, sizeof(ether), 1, fp);
		int len = ftell(fp);

		IPHEADER ip;
		int version;
		//int ihl;
		//uint8_t mask = 1;
		fread(&ip, sizeof(ip), 1, fp);
		//ihl = ip.versionAndIHL & 0x0F;
		version =  ip.versionAndIHL >> 4;
		int ipLength = NTOH2(ip.iptotalLength);

		UDP udp;
		fread(&udp, sizeof(udp), 1, fp);

		ZERG zerg;
		fread(&zerg, sizeof(zerg), 1, fp);


		int zergSourceID = NTOH2(zerg.sourceZergID);
		int zergDestinationID = NTOH2(zerg.destinationZergID);
		int zergLength  = NTOH3(zerg.totalLength);
		int sequence = NTOH4(zerg.sequenceID);
		//int sequence = ntoh(zerg.sequenceID);
		int messageLength = zergLength - 12;
		//printf("Message length of zerg message %d\n", messageLength);
		int type = zerg.versionToType & 0xF;  // This is type of message
		int zergVersion = zerg.versionToType >> 4;  // This is version
		printf("Version : %d\n", zergVersion);

		STATUSPAYLOAD status;
		//if message type = 0  it is a message and can do this
		// This is the message payload branch
		printf("Sequence : %u\n", sequence);
		printf("From : %d\n", zergSourceID);
		printf("To : %d\n", zergDestinationID);
		//printf("Ip total length %d\n", ipLength);
		//char * payloadLength = (char*) malloc((
		char * messagePayload;
		messagePayload = (char*) malloc((messageLength + 1) * sizeof(char));
		messagePayload[messageLength] = '\0';
	
		COMMAND command;
		uint16_t parameter1;
		uint8_t parameter2[4];
	
		GPS gpsDataPayload;

	//	parameter1 = NTOH2(parameter1);
	//	parameter2 = NTOH4(parameter2);
	//	int space = 2;
		switch(type)
		{
			case 0:  //regular payload
				;
				//char * messagePayload; 
				//messagePayload = (char*) malloc((messageLength + 1) * sizeof(char));
				fread(messagePayload, messageLength, 1, fp);
				//messagePayload[messageLength] = '\0';
				//printf("From: %d\n", zergSourceID);
				//printf("To: %d\n", zergDestinationID);
				printf("%s\n", messagePayload);
				free(messagePayload);
				break;
			case 1:   //status payload
				;
				//printf("This is fp before it reads in status %d\n", ftell(fp));
				fread(&status, sizeof(status), 1, fp);
				fread(messagePayload, messageLength, 1, fp);
				//printf("Status Type is %d\n", status.statusType);
				int statusType = status.statusType; 
				uint32_t  speed = NTOH4(status.speed);
				int hitPoints = NTOH3(status.hitPoints);
				int maxhp = NTOH3(status.maxHitPoints);
				printf("Name    : %s\n", messagePayload); 
				printf("Hp      : %d/%d\n", hitPoints, maxhp);
				char * breed = zergBreed(statusType);
				printf("Name    : %s\n",  breed);
				printf("Armor   : %d\n", status.armor);
				double nSpeed = convert32ToDouble(speed);
				printf("Speed   : %lfm/s\n", nSpeed);
				break;
				//printf("name is: :%s", messagePayload);
			case 2: //command payload
				;
				fread(&command, sizeof(command), 1, fp);
				int commandNum = NTOH2(command.commandField);
				char * commandWord = commandOption(commandNum);
				printf("%s", commandWord);
				if(commandNum %2 == 0){
				// Command payload only 2 bytes instead of 8
					;

				}else{
					fread(&parameter1, sizeof(parameter1), 1, fp);
					fread(&parameter2, sizeof(parameter2), 1, fp);
					parameter1 = NTOH2(parameter1);
					int nParameter2 = NTOH4(parameter2);
					double nParameter2Float = convert32ToDouble(nParameter2);
					if(commandNum == 1){  //GOTO COMMAND
						printf("   %lf  %d\n",  nParameter2Float, parameter1);
					}
					else if (commandNum == 3){ //RESERVED
						; // do nothing
					}
					else if (commandNum == 5){  
						if(parameter1 != 0){ //SETGROUP
							//True statement and should be ADD OR SUBTRACT
							printf("   %lf ADD\n", nParameter2Float);
						}else{
							printf("   %lf SUBTRACT\n", nParameter2Float);
						}
				
					}else{  //REPEAT COMMAND
						printf("   %lf\n", nParameter2Float);
					}
				}
				break;

			case 3:
				fread(&gpsDataPayload, sizeof(gpsDataPayload), 1, fp);
				uint64_t lat = ntoh64(gpsDataPayload.latitude);
				double newLat = convert64ToDouble(lat);

				uint64_t longitude = ntoh64(gpsDataPayload.longitude);
				double newLong = convert64ToDouble(longitude);
								
				uint32_t altitude = NTOH4(gpsDataPayload.altitude);
				double alt32 = convert32ToDouble(altitude);		

				uint32_t bearing = NTOH4(gpsDataPayload.bearing);
				double bear = convert32ToDouble(bearing);

				uint32_t gpsSpeed = NTOH4(gpsDataPayload.speed);
				double gpsNSpeed = convert32ToDouble(gpsSpeed);

				uint32_t accuracy = NTOH4(gpsDataPayload.accuracy);
				double acc = convert32ToDouble(accuracy);

				printf("Latitude   :  %lf degrees\n", newLat);
				printf("Longitude  :  %lf degrees\n", newLong);
				//printf("Reverse altitude in hex %x\n", altitude);
				printf("Altitude reversed and floated  :  %f fathoms\n", alt32);
				//printf("Non reversed altitude in hex %lx\n", gpsDataPayload.altitude);
				//printf("Non reversed float value of altitude %f\n", convert32ToDouble(gpsDataPayload.altitude));
				//printf("Reverse altitude in hex %lx\n", altitude);
				printf("Bearing    :  %f degrees\n", bear);
				//printf("Hex of speed reversed %x\n", gpsSpeed);
				
				printf("Speed      :  %fm/s\n", gpsNSpeed);
				printf("Accuracy   :  %fm\n", acc);
				break;
		}
		//printf("\n***********************************\n");
		int difference = endLength - ftell(fp);
		//printf("this is difference between end of packet and current position of file pointer %d\n", difference);
		if(difference != 0){
			fseek(fp, difference, SEEK_CUR);
		}		
		
	}
	fclose(fp);
	return(0);
}



long unsigned swap32(long unsigned val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}


char * zergBreed(int breedType)
{
	char * word = {'\0'};
	switch(breedType)
	{
		case 0: 
			word = "Overmind";
			break;
		case 1:
			word = "Larva";
			break;
		case 2:
			word = "Cerebrate";
			break;
		case 3:
			word = "Overlord";
			break;
		case 4:
			word = "Queen";
			break;
		case 5:
			word = "Drone";
			break;
		case 6:
			word = "Zergling";
			break;
		case 7:
			word = "Lurker";
			break;
		case 8:
			word  = "Broodling";
			break;
		case 9:
			word = "Hydralisk";
			break;
		case 10:
			word = "Guardian";
			break;
		case 11:
			word = "Scourge";
			break;
		case 12:
			word = "Ultralisk";
			break;
		case 13:
			word = "Mutalisk";
			break;
		case 14:
			word = "Defiler";
			break;
		case 15:
			word = "Devourer";
			break;
	}
	return(word);
}

char * commandOption(int instruction){
	char * word = {'\0'};
	switch(instruction)
	{
		case 0:
			word = "GET_STATUS";
			break;
		case 1:
			word = "GOTO";
			break;
		case 2:
			word = "GET_GPS";
			break;
		case 3:
			word = "RESERVED";
			break;
		case 4:
			word = "RETURN";
			break;
		case 5:
			word = "SET_GROUP";
			break;
		case 6:
			word = "STOP";
			break;
		case 7:
			word = "REPEAT";
			break;
	}
	return(word);
}


uint64_t ntoh64(uint8_t number[8])
{
	uint64_t newNumber = (((uint64_t) number[0] << 56) | ((uint64_t) number[1] << 48) | ((uint64_t) number[2] << 40) | ((uint64_t) number[3] << 32)
				 | ((uint64_t) number[4] << 24) | ((uint64_t) number[5] << 16) | ((uint64_t) number[6] << 8) 
				 | ((uint64_t) number[7]));

	return(newNumber); 
}

int fileSize(FILE *fp)
{
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	return(size);
}

int hexToDec(int x)
{
	int decimalNum = 0;
	int remainder = 0;
	int count = 0;
	while(x > 0)
	{
		remainder = x % 10;
		decimalNum = (decimalNum + remainder) * pow(16, count);
		x = x/10;
		count++;
	}
	return(decimalNum);
}

//credit to Daniel Roberts
double convert64ToDouble(uint64_t num)
{
	uint8_t sign;
	uint16_t exponent;
	uint64_t mantissa;
	double result = 0;

	sign = num >> 63;
	exponent = (num >> 52 & 0x7FF) - 1023;
	mantissa = num & 0xFFFFFFFFFFFFF;
	result = (mantissa *pow(2, -52)) + 1;
	result *= pow(1, sign) * pow(2, exponent);
	return(result);

}

double convert32ToDouble(uint32_t num)
{
	uint8_t sign, exponent;
	uint32_t mantissa;
	double result = 0;
	if(num > 0){	
		sign = num >> 31;
		exponent = (num >> 23 & 0xFF) - 127;
		mantissa = num & 0x7FFFFF;
		result = (mantissa *pow(2, -23)) + 1;
		result *= pow(1, sign) * pow(2, exponent);
	}	
	return(result);
}
