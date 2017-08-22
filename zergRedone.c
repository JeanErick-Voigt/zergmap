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

char *zergBreed(int breedType);
long unsigned swap32(long unsigned val);
char * commandOption(int instruction);
uint64_t ntoh64(uint8_t number[8]);
int fileSize(FILE *fp);
int hexToDec(int x);
double convert64ToDouble(uint64_t num);
double convert32ToDouble(uint32_t num);
void tree_insert(struct BinarySearchTree *root, double longitude, double altitude, double latitude, int id, int hp, int maxhp, int type);
struct BinarySearchTree * init_tree(double longitude, double altitude, double latitude, int id, int hp, int maxhp);
void inorder(BST *root);
int endLength(uint32_t packet, FILE *fp);
int GetVersion(FILE *fp);
//void checkTree(struct BinarySearchTree **root, double longitude, double altitude, double latitude, int id, int type, int hp, int maxhp);


int main(int argc, char *argv[])
{
	double newLong, newLat, alt32;
	newLong = -181.00;
	newLat = -91.00;
	alt32 = -1;
	// BST root
	struct BinarySearchTree *root = NULL;
	// initialize hp and maxhp to pass as parameters in the case that you don't pass that type
	int hitpoints,  maxhp;
	hitpoints = maxhp = -1;
	TOPHEADER fileHeader;
	FILE *fp = fopen(argv[1], "r");
	if(argc > 2)
		printf("ARGC: %d\n", argc);
	if (fp == NULL){
		printf("file does not exist\n");
		exit(1);
	}
	int fullSize = fileSize(fp); 
	fread(&fileHeader, sizeof(fileHeader), 1, fp);
	int count = 0;

	while(ftell(fp) < fullSize)
	{
		//printf("Beginning of while loop\n");
		count++;

		PACKETHEADER packet;
		fread(&packet, sizeof(packet), 1, fp);
		int packetLength;
		packetLength = endLength(packet.lengthDataCaptured, fp);
		
		ETHERNET ether;
		fread(&ether, sizeof(ether), 1, fp);

		int ipVersion;
		ipVersion = GetVersion(fp);
		//uint8_t version;
		//fread(&version, sizeof(version), 1, fp);
		//ipVersion = version >> 4;
		printf("This is version ip version %d\n", ipVersion);


		IPV4 internetProtocol;
		
		if(ipVersion != 4){
			fseek(fp, 39, SEEK_CUR);
		}else{
			fread(&internetProtocol, sizeof(internetProtocol), 1, fp);
			fseek(fp, -1, SEEK_CUR);
			int ipLength = NTOH2(internetProtocol.iptotalLength);
		}
		

		UDP udp;
		fread(&udp, sizeof(udp), 1, fp);
		printf("Size of udp %d\n", sizeof(udp));
		ZERG zerg;
		fread(&zerg, sizeof(zerg), 1, fp);


		int zergSourceID = NTOH2(zerg.sourceZergID);
		int zergDestinationID = NTOH2(zerg.destinationZergID);
		int zergLength  = NTOH3(zerg.totalLength);
		int sequence = NTOH4(zerg.sequenceID);
		int messageLength = zergLength - 12;
		//printf("This is zergLength %d\n", zergLength);
		int type = zerg.versionToType & 0xF;  // This is type of message
		int zergVersion = zerg.versionToType >> 4;  // This is version
		//printf("Version : %d\n", zergVersion);
		//if(root != NULL){
		//	printf("This is root id before statuspayload %d\n", root->id);
		//}
		STATUSPAYLOAD status;
		printf("From : %d\n", zergSourceID);
		printf("To : %d\n", zergDestinationID);

		GPS gpsDataPayload;

		//check to see if works just added last
		char *messagePayload = (char*) malloc((messageLength + 1) * sizeof(char));
		messagePayload[messageLength] = '\0';
		printf("This is message length %d\n", messageLength);
		switch(type)
		{
			//printf("This is message length %d\n", messageLength);
			//char *messagePayload = (char*) malloc((messageLength + 1) * sizeof(char));
			//messagePayload[messageLength] = '\0';
			//case 0:  //regular payload
			//	;
			//	char * messagePayload; 
			//	messagePayload = (char*) malloc((messageLength + 1) * sizeof(char));
			//	fread(messagePayload, messageLength, 1, fp);
			//	messagePayload[messageLength] = '\0';
				//printf("From: %d\n", zergSourceID);
				//printf("To: %d\n", zergDestinationID);
			//	printf("This is message payload --->%s\n", messagePayload);
			//	free(messagePayload);
			//	break;
			case 1:   //status payload
				;
				printf("Make hp pcap\n");
				fread(&status, sizeof(status), 1, fp);
				//printf("Read status payload\n");
				fread(messagePayload, messageLength, 1, fp);
				//printf("read message payload\n");
				int statusType = status.statusType; 
				uint32_t  speed = NTOH4(status.speed);
				hitpoints = NTOH3(status.hitPoints);
				maxhp = NTOH3(status.maxHitPoints);
				//printf("Name    : %s\n", messagePayload); 
				printf("Hp      : %d/%d\n", hitpoints, maxhp);
				//char breed[11];
				//strcpy(breed, zergBreed(statusType));
				printf("Name: %s\n", zergBreed(statusType));
				//printf("Debug statement\n");
				//printf("Name    : %s\n",  breed);
				//breed[0] = '\0';
				printf("Armor   : %d\n", status.armor);
				double nSpeed = convert32ToDouble(speed);
				printf("Speed   : %lfm/s\n", nSpeed);
				printf("Gets to break statement\n");
				break;
				//printf("name is: :%s", messagePayload);
			case 3:
				fread(&gpsDataPayload, sizeof(gpsDataPayload), 1, fp);
				uint64_t lat = ntoh64(gpsDataPayload.latitude);
				newLat = (double)convert64ToDouble(lat);

				uint64_t longitude = ntoh64(gpsDataPayload.longitude);
				printf("This is longitude %ld\n", longitude);
				newLong = (double)convert64ToDouble(longitude);
				printf("This is longitude %lf\n", newLong);

				uint32_t altitude = NTOH4(gpsDataPayload.altitude);
				alt32 = (double)convert32ToDouble(altitude);

				uint32_t bearing = NTOH4(gpsDataPayload.bearing);
				double bear = convert32ToDouble(bearing);

				uint32_t gpsSpeed = NTOH4(gpsDataPayload.speed);
				double gpsNSpeed = convert32ToDouble(gpsSpeed);

				uint32_t accuracy = NTOH4(gpsDataPayload.accuracy);
				double acc = convert32ToDouble(accuracy);

				break;
		}
		int difference = packetLength - ftell(fp);
		if(difference != 0){
			fseek(fp, difference, SEEK_CUR);
		}
		// BST information here
		printf("This is zergSourceID before conditional to see if it is null %d\n", zergSourceID);
		//checkTree(root, newLong, alt32, newLat, zergSourceID, type, hitpoints, maxhp);
		if (root == NULL){    // if checktree doesn't work revert back to this function
			root = init_tree(newLong, alt32, newLat, zergSourceID, hitpoints, maxhp);
			//printf("Root is in NULL this is new id %d\n", root->id);
		}else{
			//printf("root does not equal null %d\n", root->id);
			tree_insert(root, newLong, alt32, newLat, zergSourceID, hitpoints, maxhp, type);
		}
		//printf("This is root id before iteration of tree insert %d\n", root->id);
		//printf("This is zergsourceID %d\n", zergSourceID);
		count++;
		//printf("Current count %d\n", count);

	}
	//printf("This is final final final root id %d\n", root->id);
	inorder(root);
	printf("This is end\n");
	printf("This is amount of nodes %d\n", count);
	fclose(fp);
	return(0);
}



int GetVersion(FILE *fp)
{
	uint8_t ipversion;
	fread(&ipversion, sizeof(ipversion), 1, fp);
	return((int)ipversion >> 4);
}

void inorder(BST *root)
{
	if(root != NULL){
		inorder(root->left);
		printf("ID %d  hp %d   lat %lf \n", root->id, root->hp, fabs(root->latitude));
		inorder(root->right);
	}
}

/*void checkTree(struct BinarySearchTree **root, double longitude, double altitude, double latitude, int id, int type, int hp, int maxhp)
{
	if(*root == NULL){
		*root = init_tree(longitude, altitude, latitude, id, hp, maxhp);
		printf("root is NULL, this is new id %d\n", *root->id);
			//return(root);
	}else{
		printf("root does not equal null %d\n", *root->id);
		tree_insert(**root, longitude, altitude, latitude, id, hp, maxhp, type);
	}
	printf("End of checktree function\n");
 	//return(root);
}*/

void tree_insert(struct BinarySearchTree *root, double longitude, double altitude, double latitude, int id, int hp, int maxhp, int type)
{
	//printf("This is root id in function %d\n", root->id);
	if(id < root->id){
		//printf("ID is less\n");
		if(root->left != NULL){
			//printf("debug statement \n");
			tree_insert(root->left, longitude, altitude, latitude, id, hp, maxhp, type);
			//printf("This is root->left %d\n\n\n\n\n", root->left->id);
		}else{
			//if(root->left == NULL){
			//	printf("This is root before change\n");
			root->left = init_tree(longitude, altitude, latitude, id, hp, maxhp);
			printf("this is id after recursive call %d\n", root->left->id);

		}
	}
	else if(id > root->id){
		//printf("Id is more\n");
		if(root->right != NULL){
			tree_insert(root->right, longitude, altitude, latitude, id, hp, maxhp, type);
		}else{
			root->right = init_tree(longitude, altitude, latitude, id, hp, maxhp);
			printf("This is id %d\n", root->right->id);
		}
	}else{
		// if it is a duplicate packet it will equal id, needs to update gps or hp data
	//	tree_insert(root, longitude,altitude, latitude, id, hp, maxhp, type);
		printf("root->id %d and zergSourceID %d should be the same\n", root->id, id);
		return;
	}
}


struct BinarySearchTree *init_tree(double longitude, double altitude, double latitude, int id, int hp, int maxhp)
{
	struct BinarySearchTree *t = malloc(sizeof(struct BinarySearchTree));
	t->left = t->right = NULL;
	t->altitude = altitude;
	t->longitude = longitude;
	t->latitude = latitude;
	t->id = id;
	t->hp = hp;
	t->maxhp = maxhp;
	return(t);
}

/*struct BinarySearchTree *init_tree_hp(int hitPoints, int maxhp, int id)
{
	struct BinarySearchTree *t = NULL;
	t = malloc(sizeof(struct BinarySearchTree));
	t->left = t->right = NULL;
	t->altitude  = t->longitude = t->latitude = NULL;
	t->id = id;
	t->hp = hitPoints;
	t->maxhp = maxhp;
	return(t);
}*/


long unsigned swap32(long unsigned val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}


char *zergBreed(int breedType)
{
	char *word = malloc(11 * sizeof(char));
	word[0] = '\0';
	printf("ZERG BREED Function\n");
	printf("This is breed Type %d\n", breedType);
	switch(breedType)
	{
		case 0: 
			strcpy(word, "Overmind");
			break;
		case 1:
			strcpy(word, "Larva");
			break;
		case 2:
			strcpy(word, "Cerebrate");
			break;
		case 3:
			strcpy(word, "Overlord");
			break;
		case 4:
			strcpy(word, "Queen");
			break;
		case 5:
			strcpy(word, "Drone");
			break;
		case 6:
			printf("This is breed\n");
			strcpy(word, "Zergling");
			printf("This is word %s\n", word);
			break;
		case 7:
			strcpy(word, "Lurker");
			break;
		case 8:
			strcpy(word, "Broodling");
			break;
		case 9:
			strcpy(word, "Hydralisk");
			break;
		case 10:
			strcpy(word, "Guardian");
			break;
		case 11:
			strcpy(word, "Scourge");
			break;
		case 12:
			strcpy(word, "Ultralisk");
			break;
		case 13:
			strcpy(word, "Mutalisk");
			break;
		case 14:
			strcpy(word, "Defiler");
			break;
		case 15:
			strcpy(word, "Devourer");
			break;
	}
	printf("This is word again %s\n", word);
	return(word);
}

/*char * commandOption(int instruction){
	char * word = NULL;
	switch(instruction)
	{
		case 0:
			strcpy(word, "GET_STATUS");
			break;
		case 1:
			strcpy(word, "GOTO");
			break;
		case 2:
			strcpy(word, "GET_GPS");
			break;
		case 3:
			strcpy(word, "RESERVED");
			break;
		case 4:
			strcpy(word, "RETURN");
			break;
		case 5:
			strcpy(word, "SET_GROUP");
			break;
		case 6:
			strcpy(word, "STOP");
			break;
		case 7:
			strcpy(word, "REPEAT");
			break;
	}
	return(word);
}*/


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

int endLength(uint32_t packet, FILE *fp)
{
	int packetLength;
	packetLength = (int)packet;
	fseek(fp, packetLength, SEEK_CUR);
	int length = ftell(fp);
	fseek(fp, -packetLength, SEEK_CUR);
	return(length);
}
