#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
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
void neighbor_count_per_node(BST *array[], int *node_count, int **matrix);
void node_neighbor_removal(int row, int *node_count, int **matrix);

int main(int argc, char *argv[])
{
	FILE *fp;

	FILE *fp1;
	int option = 0;
//	char word[3] = "57";
//	printf(
	double val;
	double hp_threshold = -1.00;
	while((option = getopt(argc, argv, "h:")) != -1){
		switch(option){
			case 'h':
				val = (double)atoi(optarg);
				printf("THis is val %d\n", val);
				//printf("getopt DEBUG 1 %d\n", (int)strlen(optarg));
				for(int i = 0; i < (int)strlen(optarg); i++){
					//printf("This is optarg ---------->>>>>>>%d\n", optarg[i]);
					if(optarg[i] < 47 || optarg[i] > 57){
						//printf("This is decimal of optarg ????????????%d\n", optarg[i]);
						printf("%s is improper HP flag entry option for -h because of non integer character(s) or negative number."
								" Program will exit\n", optarg);
						exit(1);
					}
					//}else{
						//hp_threshold = val;
				}
				//printf("This is val again out of loop %d\n", val);
				//printf("This is val divided by 100 %d\n", val/100);
				hp_threshold = (double)(val/100);
				break;
			//default:
				//hp_threshold = 10;
				//printf("----------In hp_threshold\n");
				//break;
		}
	}
	if(hp_threshold < 0){ // no value was set so default is 10
		hp_threshold = .10;
	}

	printf("\nThis is final hp threshold from the command line %lf\n", hp_threshold);

	double newLong, newLat, alt32;
	newLong = -181.00;
	newLat = -91.00;
	alt32 = 0;
	int count = 0;
	int *my_count = &count;
	//printf("This is mycount %d\n", *my_count);
	*my_count += 1;
	//printf("This is mycount %d\n", *my_count);
	*my_count -= 1;
	//printf("This is mycount %d\n", *my_count);
	// BST root
	struct BinarySearchTree *root = NULL;
	// initialize hp and maxhp to pass as parameters in the case that you don't pass that type
	int hitpoints,  maxhp;
	hitpoints = maxhp = -1;
	//TOPHEADER fileHeader	
//	for(int i  = 1; i < argc; i++){
//		fp = fopen(argv[i], "r");
//		if(fp == NULL){
			//printf("file does not exist %s\n", argv[i]);
			//exit(1);
			//printf("This is check\n");

//		}else{
//			fclose(fp);
//		}
//	}
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
	if(argc < 2){
		printf("Not enough arguments to proceed with\n");
		exit(1);
	}
	for(int i = 1; i < argc; i++){
		fp = fopen(argv[i], "r");
		if(fp == NULL){
			if((argv[i][0] == '-' && argv[i][1] != 'h') || (argv[i][0] != '-' && argv[i - 1][0] != '-')){
				printf("%s is an improper command line argument\n", argv[i]);
				printf("program will exit\n");
				exit(1);
			}
		}else{
			fclose(fp);
		}
	} 
	printf("amount of argc arguments %d\n\n\n", argc);
	for(int i = 1; i < argc; i++){
		fp1 = fopen(argv[i], "r");
		if(fp1 == NULL){
			//if((argv[i][0] == '-' && argv[i][1] != 'h') || (argv[i][0] != '-' && argv[i - 1][0] != '-')){
			//	printf("%s is an improper command line argument\n", argv[i]);
			//	printf("program will exit\n");
			//	exit(1);
			//}
			continue;
		}
		//TOPHEADER fileHeader;
		//printf("out of command line check\n");
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
			printf("Size of udp %ld\n", sizeof(udp));
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
	if(*my_count == 0){
		printf("This is before I exit\n");
		exit(1);
	}
	BST **nodes = malloc(*my_count * sizeof(struct BinarySearchTree *));
	for(int i = 0; i < *my_count; i++){
		nodes[i] = malloc(sizeof(struct BinarySearchTree));
	}
	init_array(nodes, root, my_count);
	// printf the hp's below the certain percentage
		printf("Below health (%lf%%)\n", (hp_threshold * 100));
		for(int i = 0; i < *my_count; i++){
			if(nodes[i]-> hp < (hp_threshold * nodes[i]->maxhp) && nodes[i]->hp >= 0){
				printf("zerg #%d, hp: %d, maxhp: %d\n", nodes[i]->id, nodes[i]->hp, nodes[i]->maxhp);
			}
		}
	//look above for this

	double distance;
	//printf("This is after distane declared\n");
/////////////////////////////////////////////////////// 	where i left off below
	//printf("Before adjacency matrix\n");
	int **adjacency = malloc((*my_count + 1) * sizeof(int *));
	int **temp_adjacency = malloc((*my_count + 1) * sizeof(int *));
	for(int i = 0; i < *my_count + 1; i++){
		adjacency[i] = (int *)malloc((*my_count + 2) * sizeof(int));
		temp_adjacency[i] = (int *)malloc((*my_count + 2) * sizeof(int));
	// this is statically allocated 2d array below
	//int adjacency[*my_count + 1][*my_count + 2];
	}
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
					adjacency[i][j] = -5;
					//printf("Debug2\n");
				}
			}
			else if(i != 0 && j == 0){
				//printf("DEBUG2\n");
				adjacency[i][j] = nodes[i-1]->id;
				//printf("%-6d", nodes[i-1]->id);
			}
			else if(j == *my_count + 1){
				adjacency[i][j] = -5;
			}else{
				//print connection
				//example of format
				//printf("i:%-4d", i); format example
				//printf("This is node i and node j latitude %lf and logitude %lf and altitude %lf %lf %lf %lf\n", nodes[i-1]->longitude, nodes[i-1]->latitude, 
				//	nodes[i - 1]->altitude, nodes[j-1]->longitude, nodes[j-1]->latitude, nodes[j-1]->altitude);
				distance = haversine_formula(nodes[i-1]->longitude, nodes[i-1]->latitude, nodes[i-1]->altitude,
					nodes[j-1]->longitude, nodes[j-1]->latitude, nodes[j-1]->altitude);
				if(distance >= (1.25000 * .9144) && distance <= 15.00000){
					adjacency[i][j] = 1;
				//	printf("This is distance %lf\n", distance);
				}else{
					adjacency[i][j] = 0;
				}
				//printf("%-4d", adjacency[i][j]);
			}
		}
		printf("\n");
		//printf("nodes %d, latitude %lf, and longitude %lf\n", nodes[i]->id, nodes[i]->latitude, nodes[i]->longitude);
	} 
	printf("Before the print for adjacency matrix\n");
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
//check the neighbor // comment out to check if neighbor function works
	
/*	for(int i = 0; i < *my_count; i++){
		int count = 0;
		for(int j = 0; j < *my_count; j++){
			if(adjacency[i+1][j+1] == 1){
				count++;
			}else{
				;
			}
		}
		if(count >= 2){
			nodes[i]->connected = count;
			printf("Node %d is %d\n", nodes[i]->id, nodes[i]->connected);
		}else{
			nodes[i]->connected = 0;
			for(int j = 1; j < *my_count + 1; j++){
				adjacency[i+1][j] = -1;
				adjacency[j][i+1] = -1;
			} 
			printf("Node %d is not connected\n", nodes[i]->id);
		}
		printf("Debug statement\n");
	}
*/	//printf("out of loop\n");


//////// Above commmented out to check if neighbor function works
neighbor_count_per_node(nodes, my_count, adjacency);
//print updated node adjacency after function call

	for(int i = 0; i < *my_count + 1; i++){
		for(int j = 0; j < *my_count + 2; j++){
			printf("%-6d", adjacency[i][j]);
		}
		printf("\n");
	}

//sets now
	int *set_array = malloc(*my_count * sizeof(int));//for(int i = 0
	for(int i = 0; i < *my_count; i++){
		set_array[i] = i;
		//printf("%d\n", set_array[i]);
	}
	// go through set and update for reachability based on array connections.
	for(int i = 0; i < *my_count; i++){
		for(int j = 0; j < *my_count; j++){
			if((i + 1) == (j + 1)){
				continue;
			}else{
				if(adjacency[i+1][j+1] == 1){
					if(set_array[i] < set_array[j]){
						set_array[j] = set_array[i];
						//printf("This is set_array j after reset value %d\n", set_array[j]);
					}
					else if(set_array[i] > set_array[j]){
						set_array[i] = set_array[j];
					}else{
						continue;
					}
				}else{
					continue;
				}
			}
		}
	}

	printf("These are updated sets\n");
	for(int i = 0; i < *my_count; i++){
		printf("%d", set_array[i]);
	}

	// remove zergs based on reachability and or sets
	printf("\n This is after sets printed out\n");

/////////////////////////////////////////	look above

	printf("this is end\n");
	//printf("This is amount of nodes %d\n", *my_count);
	printf("This is last statement\n");
	if(fp1 != NULL){
		fclose(fp1);
	}
	printf("After the fclose ----------------\n\n");
	return(0);
}

void neighbor_count_per_node(BST *array[], int *node_count, int **matrix)
{

	double removal, total_removal;
	int total_qualified_nodes = 0;
	int neighbor_count = 0;
	int node_removal = 1;
	total_removal = 0;
	while((total_removal / *node_count) < .50 && node_removal > 0){
	//removal =  0
		for(int row = 0; node_removal != 0 && row < *node_count; row++){
			removal = 0;
			for(int column = 0;  column < *node_count; column++){
				if(matrix[row+1][column+1] == 1){
					neighbor_count++;
					//total_removal += removal;
				}
			}
			array[row]->connected = neighbor_count;
			//printf("This is array row count %d\n", array[row]->connected);
			//printf("This is DEBUG OF function: neighbor count %d\n", neighbor_count);
			//printf("This is array to be removed %d\n", array[row]->id);
			if(neighbor_count < 2){
				removal++;
				total_removal++;
			}
			neighbor_count = 0;
			if(removal > 0){
				if(((removal / *node_count) < .50) && ((total_removal / *node_count) < .50)){
			//		printf("Removing array %d\n", array[row]->id);
					node_neighbor_removal(row + 1, node_count, matrix);
				}else{
					printf("To many nodes to remove, might want to add nodes instead\n");
					node_removal = 0; 
					break;
				}
			}
		}
		//printf("DEBUG before for looop\n");
		for(int i = 0; i < *node_count; i++){
			if(array[i]->connected >=2){
				total_qualified_nodes++;
			}
		}
		//printf("This is total_qualified_nodes %d\n", total_qualified_nodes);
		if(total_qualified_nodes == *node_count){
			printf("All nodes have at least two neighbors");
			node_removal = 0;
			break;
		}else{
			total_qualified_nodes = 0;
		}
	}
}
	/*static double total_removal;
	double removal;
	int nodes = 1;
	int neighbor_check;
	//int removal_num = 0;
	while(nodes > 0 && (total_removal / *node_count) > .50 ){
		neighbor_check = 0;
		for(int i = 0; i < *node_count; i++){
			int count = 0;
			//printf("This is count variable %d\n");
			for(int j = 0; j < *node_count; j++){
				if(matrix[i+1][j+1] == 1){
					printf("This is matrix variable %d\n", matrix[i+1][j+1]);
					count++;
					//removal_num++;
				}else{
					;
				}
			}
			array[i]->connected = count;
			printf("This is nodes connectivity %d for array[i]: %d\n", array[i]->connected, array[i]->id);
			//printf("Array node %d and amount of edges %d\n", array[i]->id, array[i]->connected);
			//printf("node connectivity:  %d\n", array[i]->connected);
		}
		for(int i = 0; i < *node_count; i++){
			if(array[i]->connected >= 2){
				neighbor_check++;
			}
		}
		if(neighbor_check == *node_count){
			nodes = 0;
			printf("Nodes should not be 0 anymore\n");
			break;
		}else{
			printf("Before Removal statement\n");
			removal = node_neighbor_removal(array, node_count, matrix);
			if(removal >= 0 && removal <= .001){
				nodes = 0;
				printf("No more nodes to remove\n");
				break;
			}else{
				printf("This is removal number %lf\n", removal);
				break;
			}
		}	//return removal_num;
		printf("After last big else in function after removal run\n");
	}
}
*/
void node_neighbor_removal(int row, int *node_count, int **matrix)
{
//	static int function_call = 0

	//for(int row = 0; row < *node_count; row++){
	//	int count = 0;
	//	for(int column = 0; column < *node_count; column ++){
	//		if(matrix[row+1][column+1] == 1){
	//			count++;
	//		}else{
	//			;
	//		}
	//	}
	//	array[row]->connected = count;
	//}
	int num;
	for(int column = 0; column < *node_count; column++){
		matrix[row][column+1] = -1;
		matrix[column+1][row] = -1;
	}
	//return(removal);

}  // added this stuff in the end
//	printf("Amount to be removed %lf\n", removal);
//	printf("This is node count %d\n", *node_count);
//	printf("This is removal / *node_count: %.3lf\n", removal / *node_count);
//	if(((removal / *node_count) >= .5) || ((total_removed / *node_count) >= .5)){
//		printf("All nodes do not have enough neighbors.  But removal would cause more than 50%% of nodes to be removed\n");
//		printf("Might want to add nodes instead\n");
//	}else if(((removal / *node_count) > 0) && ((total_removed / *node_count) >= 0)){
/*		for(int i = 0; i < *node_count; i++){
			if(array[i]-> connected < 2 && array[i]->connected > 0){
				//array[i]->connected = 0;
				for(int j = 0; j < *node_count + 1; j++){
					array[i]->connected = 0;
					matrix[i+1][j + 1] = -1;
					if(j < *node_count){
						matrix[j][i+1] = -1;
					}else{
						break;
					}
				}
			}
		}
		printf("This is print statement\n");
	}else{
		printf("This is final else statement\n");
		printf("This is removal numbers for total removed %lf\n", total_removed);
		;
		//do nothing
		//return(removal);
	}
	printf("Function call %d\n", function_call);
	return(removal);
}
*/
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
	//printf("This is in function\n");
	if(root != NULL){
		//printf("BEFORE COUNT \n");
		//(*count) += 1;
		//printf("This is count %d\n", *count);
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
	//if(*my_count == 0){
	//	printf("Error not enough nodes\n");
	//}
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










/////
//sets now
	int *set_array = malloc(*my_count * sizeof(int));//for(int i = 0
	for(int i = 0; i < *my_count; i++){
		set_array[i] = i;
		//printf("%d\n", set_array[i]);
	}
	// go through set and update for reachability based on array connections.
	for(int i = 0; i < *my_count; i++){
		for(int j = 0; j < *my_count; j++){
			if((i + 1) == (j + 1)){
				continue;
			}else{
				if(adjacency[i+1][j+1] == 1){
					if(set_array[i] < set_array[j]){
						set_array[j] = set_array[i];
						//printf("This is set_array j after reset value %d\n", set_array[j]);
					}
					else if(set_array[i] > set_array[j]){
						set_array[i] = set_array[j];
					}else{
						continue;
					}
				}else{
					continue;
				}
			}
		}
	}

	printf("These are updated sets\n");
	for(int i = 0; i < *my_count; i++){
		printf("%d", set_array[i]);
	}

	// remove zergs based on reachability and or sets
	printf("\n This is after sets printed out\n");
