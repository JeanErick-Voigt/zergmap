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
void inorder(BST *root, int *count);
int endLength(uint32_t packet, FILE *fp);
int GetVersion(FILE *fp);
double haversine_formula(double longitude1, double latitude1, double altitude1, double longitude2, double latitude2, double altitude2);
//void checkTree(struct BinarySearchTree **root, double longitude, double altitude, double latitude, int id, int type, int hp, int maxhp);
void init_array(BST **nodes, BST *root, int *count);


int main(int argc, char *argv[])
{
	FILE *fp;
	FILE *fp1;
	double newLong, newLat, alt32;
	newLong = -181.00;
	newLat = -91.00;
	alt32 = 0;
	int count = 0;
	int *my_count = &count;
	printf("This is mycount %d\n", *my_count);
	*my_count += 1;
	printf("This is mycount %d\n", *my_count);
	*my_count -= 1;
	printf("This is mycount %d\n", *my_count);
	// BST root
	struct BinarySearchTree *root = NULL;
	// initialize hp and maxhp to pass as parameters in the case that you don't pass that type
	int hitpoints,  maxhp;
	hitpoints = maxhp = -1;
	//TOPHEADER fileHeader	
	for(int i  = 1; i < argc; i++){
		fp = fopen(argv[i], "r");
		if(fp == NULL){
			printf("file does not exist %s\n", argv[i]);
			exit(1);
		}else{
			fclose(fp);
		}
	}
//	FILE *fp = fopen(argv[i], "r");
//	if(argc > 2)
//		printf("ARGC: %d\n", argc);
	//if (fp == NULL){
	//	printf("file does not exist\n");
	//	exit(1);
	//}

	
	//int fullSize = fileSize(fp); 
	//fread(&fileHeader, sizeof(fileHeader), 1, fp);
	//int count = 0;

	int fullSize;
	for(int i = 1; i < argc; i++){
		fp1 = fopen(argv[i], "r");
		//TOPHEADER fileHeader;
		fullSize = fileSize(fp1);
		TOPHEADER fileHeader;
		fread(&fileHeader, sizeof(fileHeader), 1, fp1);
		while(ftell(fp1) < fullSize)
		{
			//TOPHEADER fileHeader;
			//fread(&fileHeader, sizeof(fileHeader), 1, fp);
			//printf("Beginning of while loop\n");

			PACKETHEADER packet;
			fread(&packet, sizeof(packet), 1, fp1);
			int packetLength;
			packetLength = endLength(packet.lengthDataCaptured, fp1);
		
			ETHERNET ether;
			fread(&ether, sizeof(ether), 1, fp1);

			int ipVersion;
			ipVersion = GetVersion(fp1);
			//uint8_t version;
			//fread(&version, sizeof(version), 1, fp);
			//ipVersion = version >> 4;
			printf("This is version ip version %d\n", ipVersion);


			IPV4 internetProtocol;
		
			if(ipVersion != 4){
				fseek(fp1, 39, SEEK_CUR);
			}else{
				fread(&internetProtocol, sizeof(internetProtocol), 1, fp1);
				fseek(fp1, -1, SEEK_CUR);
				int ipLength = NTOH2(internetProtocol.iptotalLength);
			}
		

			UDP udp;
			fread(&udp, sizeof(udp), 1, fp1);
			printf("Size of udp %d\n", sizeof(udp));
			ZERG zerg;
			fread(&zerg, sizeof(zerg), 1, fp1);


			int zergSourceID = NTOH2(zerg.sourceZergID);
			int zergDestinationID = NTOH2(zerg.destinationZergID);
			int zergLength  = NTOH3(zerg.totalLength);
			int sequence = NTOH4(zerg.sequenceID);
			int messageLength = zergLength - 12;
			int type = zerg.versionToType & 0xF;  // This is type of message
			int zergVersion = zerg.versionToType >> 4;  // This is version
			STATUSPAYLOAD status;
			printf("From : %d\n", zergSourceID);
			printf("To : %d\n", zergDestinationID);

			GPS gpsDataPayload;

			char *messagePayload = (char*) malloc((messageLength + 1) * sizeof(char));
			messagePayload[messageLength] = '\0';
			//printf("This is message length %d\n", messageLength);
			switch(type)
			{
				case 1:   //status payload
					;
					//printf("Make hp pcap\n");
					fread(&status, sizeof(status), 1, fp1);
					fread(messagePayload, messageLength, 1, fp1);
					int statusType = status.statusType; 
					uint32_t  speed = NTOH4(status.speed);
					hitpoints = NTOH3(status.hitPoints);
					maxhp = NTOH3(status.maxHitPoints);
					printf("Hp      : %d/%d\n", hitpoints, maxhp);
					printf("Name: %s\n", zergBreed(statusType));
					//printf("Armor   : %d\n", status.armor);
					double nSpeed = convert32ToDouble(speed);
					//printf("Speed   : %lfm/s\n", nSpeed);
					//printf("Gets to break statement\n");
					break;
				case 3:
					fread(&gpsDataPayload, sizeof(gpsDataPayload), 1, fp1);
					uint64_t lat = ntoh64(gpsDataPayload.latitude);
					newLat = (double)convert64ToDouble(lat);

					uint64_t longitude = ntoh64(gpsDataPayload.longitude);
					//printf("This is longitude %ld\n", longitude);
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
			int difference = packetLength - ftell(fp1);
			if(difference != 0){
				fseek(fp1, difference, SEEK_CUR);
			}
			// BST information here
			if (root == NULL){    // if checktree doesn't work revert back to this function
				root = init_tree(newLong, alt32, newLat, zergSourceID, hitpoints, maxhp);
			}else{
				tree_insert(root, newLong, alt32, newLat, zergSourceID, hitpoints, maxhp, type);
			}

		}
	}

	inorder(root, my_count);
	BST **nodes = malloc(*my_count * sizeof(struct BinarySearchTree *));
	for(int i = 0; i < *my_count; i++){
		nodes[i] = malloc(sizeof(struct BinarySearchTree));
	}
	init_array(nodes, root, my_count);
	double distance;
	//printf("This is after distane declared\n");
///////////////////////////////////// 	where i left off below
	//printf("Before adjacency matrix\n");
	int adjacency[*my_count + 1][*my_count + 2];
	for(int i = 0; i < *my_count + 1; i++){
		printf("In FOOOOORRRR LOOP %d\n", *my_count);
		//if(i == 0){
			//printf("     "
		//}else{
			//printf("Node %-6d", nodes[i]->id);
		for(int j = 0;  j < *my_count + 2; j++){
			//printf("This is  i value %d\n", i);
			if(i == 0 && j == 0){		
				adjacency[0][0] = -5;
				//printf("      ");
			}
			else if(i == 0 && j != 0){
				//printf("DEBUG %d", j);
				//adjacency[i][j] = nodes[j-1]->id;
				if(j < *my_count + 1){
					adjacency[i][j] = nodes[j-1]->id;
					//printf("%-6d", nodes[j-1]->id);
				}else{
					adjacency[i][j] = -1;
					//printf("Debug2\n");
				}
			}
			else if(i != 0 && j == 0){
				//printf("DEBUG2\n");
				adjacency[i][j] = nodes[i-1]->id;
				//printf("%-6d", nodes[i-1]->id);
			}
			else if(j == *my_count + 1){
				adjacency[i][j] = -1;
			}else{
				//print connection
				//example of format
				//printf("i:%-4d", i); format example
				distance = haversine_formula(nodes[i-1]->longitude, nodes[i-1]->latitude, nodes[i-1]->altitude,
					nodes[j-1]->longitude, nodes[j-1]->latitude, nodes[j-1]->altitude);
				if(distance >= (1.25000 * .9144) && distance <= 15.00000){
					adjacency[i][j] = 1;
				}else{
					adjacency[i][j] = 0;
				}
				//printf("%-4d", adjacency[i][j]);
			}
		}
		printf("\n");
		//printf("nodes %d, latitude %lf, and longitude %lf\n", nodes[i]->id, nodes[i]->latitude, nodes[i]->longitude);
	} 

	for(int i = 0; i < *my_count + 1; i++){
		for(int j = 0; j < *my_count + 2; j++){
			if(i == 0 || j == 0){
				printf("%-6d", adjacency[i][j]);
			}else{
				printf("%-6d", adjacency[i][j]);
			}
		} 
		printf("\n");
	} 

//////////////////////////	look above

	distance = haversine_formula(root->longitude, root->latitude, root->altitude, root->left->longitude, root->left->latitude, root->left->altitude);
	printf("This is distance between two points %lf\n", distance);
	printf("this is end\n");
	printf("This is amount of nodes %d\n", *my_count);
	fclose(fp1);
	return(0);
}


void init_array(BST **nodes, BST *root, int *count)
{
	static int i = 0;
	if(root != NULL){
		init_array(nodes, root->left, count);
		nodes[i++] = root;
		
		init_array(nodes, root->right, count);
	}
}

int GetVersion(FILE *fp)
{
	uint8_t ipversion;
	fread(&ipversion, sizeof(ipversion), 1, fp);
	return((int)ipversion >> 4);
}

double haversine_formula(double longitude1, double latitude1, double altitude1, double longitude2, double latitude2, double altitude2)
{
	double distance;
	double radius;
	radius = 6371 * 1000;
	double lat_change;
	lat_change = ((M_PI/180) * (latitude2 - latitude1));
	lat_change = sin(lat_change/2) * sin(lat_change/2);
	//printf("This is latitude2 %lf\n", latitude2);
	//printf("This is lat_change %lf\n", lat_change);
	double long_change;
	long_change = ((M_PI/180) * (longitude2 - longitude1));
	long_change = sin(long_change/2) * sin(long_change/2);
	//printf("This is difference of longitude %lf\n", longitude2 - longitude1);
	//printf("This is difference of long divided by 2 %lf\n", (longitude2 - longitude1)/2);
	//printf("This is long change %lf\n", long_change);
	double inside_radical;
	//printf("This is inside radical %lf\n", inside_radical);
	inside_radical = lat_change + (cos(latitude1) * cos(latitude2) * long_change);
	//printf("This is inside radical %lf\n", inside_radical);
	distance = (2 * radius) * asin(sqrt(inside_radical));
	double altitude_change;
	altitude_change = fabs(altitude2 - altitude1);
	//change altitude to meters from fathoms
	altitude_change = altitude_change * ((6 * 12)/39.37);
	double true_distance; 
	true_distance = sqrt((distance * distance) + (altitude_change * altitude_change));
	return true_distance;

}

void inorder(BST *root, int *count)
{
	//static *count;
	//printf("This is count %d\n", *count);
	//printf("This is inorder function");
	printf("This is in function\n");
	if(root != NULL){
		//printf("BEFORE COUNT \n");
		//(*count) += 1;
		printf("This is count %d\n", *count);
		//printf("This is count %d\n", *count);
		inorder(root->left, count);
		(*count) += 1;
		//if(fabs(root->latitude) <= 90)){
		printf("ID %d", root->id);
		if(fabs(root->latitude) <= 90){
			printf(" lat %lf", root->latitude);
		}
		if(fabs(root->longitude) <= 180){
			printf(" long %lf", root->longitude);
		}
		if(root->hp > 0){
			printf(" maxhp %d", root->maxhp);
		}
		if(root->maxhp > 0){
			printf(" hp %d", root->hp);
		}else{
			printf("\n");
		}
		printf("\n");

		
		inorder(root->right, count);
	}
	//printf("This is value of count %d\n", *count);
}

void tree_insert(struct BinarySearchTree *root, double longitude, double altitude, double latitude, int id, int hp, int maxhp, int type)
{
	if(id < root->id){
		if(root->left != NULL){
			tree_insert(root->left, longitude, altitude, latitude, id, hp, maxhp, type);
		}else{
			root->left = init_tree(longitude, altitude, latitude, id, hp, maxhp);
			printf("this is id after recursive call %d\n", root->left->id);

		}
	}
	else if(id > root->id){
		if(root->right != NULL){
			tree_insert(root->right, longitude, altitude, latitude, id, hp, maxhp, type);
		}else{
			root->right = init_tree(longitude, altitude, latitude, id, hp, maxhp);
			//printf("This is id %d\n", root->right->id);
		}
	}else{
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
