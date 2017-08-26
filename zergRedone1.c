#include <netinet/in.h>
#include <byteswap.h>
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

//char *zergBreed(int breedType);
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
int two_neighbors_per_node(int *node_count, int **matrix);
void node_neighbor_removal(int row, int *node_count, int **matrix);
void make_set_array(int *set_array, int *node_count, int **matrix, BST *node[]);
int find_largest_set(int *array, int *node_count);
void print_matrix(int **matrix, int *node_count);
int remove_smallest_set(int *set_array, int set_pos, BST *node[], int *node_count, int **matrix);
int remove_node(int *node_count, BST *nodes[], int row, int **matrix);
int which_node_to_remove(int *node_count, BST *nodes[], int **matrix);
void print_hp(double hp_threshold, BST *nodes[], int *node_count);

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
	//			printf("THis is val %d\n", val);
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


	double newLong, newLat, alt32;
	newLong = -181.00;
	newLat = -91.00;
	alt32 = 0;
	int count = 0;
	int *my_count = &count;
	struct BinarySearchTree *root = NULL;
	// initialize hp and maxhp to pass as parameters in the case that you don't pass that type
	int hitpoints,  maxhp;
	hitpoints = maxhp = -1;

	

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
	for(int i = 1; i < argc; i++){
		fp1 = fopen(argv[i], "r");
		if(fp1 == NULL){
			continue;
		}
		fullSize = fileSize(fp1);
		TOPHEADER fileHeader;
		fread(&fileHeader, sizeof(fileHeader), 1, fp1);
		while(ftell(fp1) < fullSize)
		{

			PACKETHEADER packet;
			fread(&packet, sizeof(packet), 1, fp1);
			int packetLength;
			packetLength = endLength(packet.lengthDataCaptured, fp1);

			ETHERNET ether;
			fread(&ether, sizeof(ether), 1, fp1);

			int ipVersion;
			ipVersion = GetVersion(fp1);

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

			GPS gpsDataPayload;
			COMMAND command;
			uint16_t parameter1;
			uint8_t parameter2[4];

			char *messagePayload = (char*) malloc((messageLength + 1) * sizeof(char));
			messagePayload[messageLength] = '\0';

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
					//printf("Hp      : %d/%d\n", hitpoints, maxhp);
					//printf("Name: %s\n", zergBreed(statusType));
					//printf("Armor   : %d\n", status.armor);
					double nSpeed = convert32ToDouble(speed);
					//printf("Speed   : %lfm/s\n", nSpeed);
					//printf("Gets to break statement\n");
					break;

				case 3:
					fread(&gpsDataPayload, sizeof(gpsDataPayload), 1, fp1);
					uint64_t lat = ntoh64(gpsDataPayload.latitude);
					newLat = (double)convert64ToDouble(lat);
					printf("This is latitude %lf\n", newLat);
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
				
				default:
					fseek(fp1, messageLength, SEEK_CUR);
					printf("%d is an incorrect payload type.\n", type);
					
			}
			int difference = packetLength - ftell(fp1);
			if(difference != 0){
				fseek(fp1, difference, SEEK_CUR);
			}
			// BST information here
			if (root == NULL){    // if checktree doesn't work revert back to this function
				root = init_tree(newLong, alt32, newLat, zergSourceID, hitpoints, maxhp);
			}else{
				if(type == 1 || type == 3){
					tree_insert(root, newLong, alt32, newLat, zergSourceID, hitpoints, maxhp, type);
				}
			}
		}
	}

	inorder(root, my_count);
	if(*my_count == 0){
		printf("No nodes present in pcaps. Program will exit\n");
		exit(1);
	}
	BST **nodes = malloc(*my_count * sizeof(struct BinarySearchTree *));
	for(int i = 0; i < *my_count; i++){
		nodes[i] = malloc(sizeof(struct BinarySearchTree));
	}
	init_array(nodes, root, my_count);
	////////////////////// HP print going to try and make a function	
		printf("Below health (%lf%%)\n", (hp_threshold * 100));
		for(int i = 0; i < *my_count; i++){
			printf("This is node %d,  hp %d nodes maxhp %d and hp_threshold %lf\n", nodes[i]->id, nodes[i]->hp, nodes[i]->maxhp,
				hp_threshold);
			if(nodes[i]-> hp < (hp_threshold * nodes[i]->maxhp) && nodes[i]->hp >= 0){
				printf("REMOVED ZERG #%d, hp: %d, maxhp: %d\n", nodes[i]->id, nodes[i]->hp, nodes[i]->maxhp);
			}
		}
	printf("printing hp below debug debug\n\n");
	print_hp(hp_threshold, nodes, my_count);
	//////////////// HP print gonna try to make a function look above
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
		for(int j = 0;  j < *my_count + 2; j++){
			//printf("This is  i value %d\n", i);
			if(i == 0 && j == 0){
				adjacency[0][0] = -5;
				//printf("      ");
			}
			else if(i == 0 && j != 0){
				//printf("DEBUG %d", j);
				//adjacency[i][j] = nodes[j-1]->id;
				if(j < *my_count + 1 && fabs(nodes[j-1]->altitude) <= 6160 && fabs(nodes[j-1]->latitude) <= 90
				    && fabs(nodes[j-1]->longitude) <= 180){
					adjacency[i][j] = nodes[j-1]->id;
					//printf("%-6d", nodes[j-1]->id);
				}else{
					adjacency[i][j] = -5;
					//printf("Debug2\n");
				}
			}
			else if(i != 0 && j == 0){
				//printf("DEBUG2\n");
				if(fabs(nodes[i-1]->altitude) <= 6160 && fabs(nodes[i-1]->latitude) <= 90
				     && fabs(nodes[i-1]->longitude) <= 180){
					adjacency[i][j] = nodes[i-1]->id;
				}else{
					adjacency[i][j] = -5;
				}
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
				printf("This is node %d longitude %lf, latitude %lf and altitude %lf\n", nodes[j-1]->id, nodes[j-1]->longitude, nodes[j-1]->latitude, nodes[j-1]->altitude);
				printf("This is node %d longitude %lf, latitude %lf and altitude %lf\n", nodes[i-1]->id, nodes[i-1]->longitude, nodes[i-1]->latitude, nodes[i-1]->altitude);
				printf("This is distance between node[i] %d and node[j] %d --> %lf\n", nodes[j-1]->id, nodes[i-1]->id, distance);
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
	//printf("Before the print for adjacency matrix\n");
	//print_matrix(adjacency, my_count);

//////// Above commmented out to check if neighbor function works
	//neighbor_count_per_node(nodes, my_count, adjacency);
	// move neigh_count after set
	print_matrix(adjacency, my_count);

//sets now
	int removed, neighbors;
	int *set_array = malloc(*my_count * sizeof(int));//for(int i = 0
// try the function instead of this above to set array
	printf("\n");
	print_hp(hp_threshold, nodes, my_count);
	make_set_array(set_array, my_count, adjacency, nodes);
	int set_pos;
	set_pos = find_largest_set(set_array, my_count); //this returns set position
	printf("this is set position %d\n");
	removed = remove_smallest_set(set_array, set_pos, nodes, my_count, adjacency);
	//printf("Set to be removed\n");
	printf("This is removed variable %d\n", removed);
	neighbors = 1;
	int removal_node;
	while(neighbors == 1){
		neighbors = two_neighbors_per_node(my_count, adjacency);
		
		if(neighbors != 1){
			break;
		}
		removal_node = which_node_to_remove(my_count, nodes, adjacency);
		if(removal_node == -1){
			neighbors = removal_node;
			printf("to many nodes to remove\n");
		}
	}
//	printf("This is neighbors variable %d and which node to remove %d\n", neighbors, removal_node);
	if(removed == 0 && neighbors == 1){
		for(int i = 0; i < *my_count; i++){
			if(nodes[i]-> removed == -1){
				printf("ZERG ID #%d removed\n", nodes[i]->id);
			}else{
				continue;
			}
		}
	}
	else if(removed == 0){
		for(int i = 0; i < *my_count; i++){
			if(nodes[i]->removed_from_set == 1){
				printf("Zerg ID #%d reomoved\n", nodes[i]->id);
			}
		}
	}
	else if(removed == 1 && neighbors == 0){
		// may want to try to check for disjoint sets here
		printf("All zerg are in position ");
	}
	else if(removed == 1 && neighbors == 2){
		printf("Some were removed and balanced now\n");
		for(int i = 0; i < *my_count; i++){
			if(nodes[i]->removed == -1){
				printf("Zerg ID #%d removed\n", nodes[i]->id);
			}
		}
	}
	printf("These are LOW ZERG DEBUG DEBUG \n");
	print_hp(hp_threshold, nodes, my_count);
	printf("\n");
	print_matrix(adjacency, my_count); 

	//printf("These are updated sets\n");
	//for(int i = 0; i < *my_count; i++){
	//	printf("%d", set_array[i]);
	//}

	// remove zergs based on reachability and or sets
	//printf("\n This is after sets printed out\n");

/////////////////////////////////////////	look above

	printf("this is end\n");
	//printf("This is amount of nodes %d\n", *my_count);
	printf("This is last statement\n");
	if(fp1 != NULL){
		fclose(fp1);
	}
	//printf("After the fclose ----------------\n\n");
	return(0);
}

void print_matrix(int **matrix, int *node_count)
{
	for(int i = 0; i < *node_count + 1; i++){
		for(int j = 0; j < *node_count + 2; j++){
			printf("%-6d", matrix[i][j]);
		}
		printf("\n");
	}
}

int find_largest_set(int *array, int *node_count)
{
	int high_set, reverse_value, count, forward_value, set_pos;
	count = 0;
	high_set = 1;
	set_pos = -1;
	reverse_value = 0;
	for(int index = 0; index < *node_count; index++){
		if( index > 0){
			reverse_value = index;
		}
		for(; reverse_value >= 0; reverse_value--){
			if(index == 0){
				count = 0;
				break;
			}
			else if(array[index] == array[reverse_value]){
				count = 0;
				break;
			}
		}
		for(forward_value = 0; forward_value < *node_count; forward_value++){

			if(array[index] == array[forward_value]){
				count++;
			}else{
				continue;
			}
		}
		if(high_set < count){
			high_set = count;
			set_pos = array[index];
		}
	}
	//printf("This is set with most matches %d with count of %d\n", set_pos, high_set);
	return(0);
}

void print_hp(double hp_threshold, BST *nodes[], int *node_count)
{
	printf("Below health (%lf%%)\n", (hp_threshold * 100));
	for(int i = 0; i < *node_count; i++){
		if(nodes[i]->hp < (hp_threshold * nodes[i]->maxhp) && nodes[i]->hp >= 0){
			printf("REMOVED ZERG# %d, hp: %d, maxhp: %d\n", nodes[i]->id, nodes[i]->hp, nodes[i]->maxhp);
		}
	}
	printf("\n\n");
}

int two_neighbors_per_node(int *node_count, int **matrix)
{

	//double removal, total_removal;
	//int return_val = -2;
	//int total_qualified_nodes = 0;
	int node_removal = 1;
	int good_row;
	int neighbor_count = 0;
	//int node_removal = 1;
	//int removal_check;
	//total_removal = 0;
	//removal = 0;
	while( node_removal > 0){
		for(int row = 0; row < *node_count; row++){
			neighbor_count = 0;
			for(int column = 0; column < *node_count; column++){
				if(matrix[row+1][column+1] == 1){
					neighbor_count++;
				}
			}
			if(neighbor_count >= 2){
				good_row++;
			}else if(neighbor_count == 1){
				printf("neighbor count is 1\n");
				return(1);
			}
		}
		if(good_row == *node_count){
			node_removal = 0;
			return(0);
		}
		else if(((double)(good_row / *node_count)) > .50){
			printf("some nodes removed\n");
			node_removal = 0;
			return(2);
		}else{
			printf("No connections at all\n");
			node_removal = 0;
			return(-1);
		}
	}
	return(-1);
}

/*			array[row]->connected = neighbor_count;
			//neighbor_count = 0;
			printf("This is neighbor check\n");
			if(array[row]->connected == 1 || neighbor_count == 1){
				neighbor_count = 0;
				printf("This is before removal\n");
				removal++;
				total_removal++;
				return_val = check_for_removal(removal, total_removal, node_count, array, row, matrix);
				printf("This is return value after check for removal function %d\n", return_val);
				removal = 0;
				if(return_val == 0){
					node_removal = 0;
				}
			}else{
				continue;
			}
		}
		int total_count = 0;
		int count;
		for(int row = 0; return_val != -1 && row < *node_count; row++){
			count = 0;
			printf("count is at 0 \n");
			for(int i = 0; i < *node_count; i++){
				printf("This is array i connected number %d and id %d\n", array[row]->connected, array[row]->id);
				if(matrix[row+1][i+1] == 1){
					//printf("going for another go around\n");
					if(count < 2){
						count++;
					}
					//printf("number of connections for this node based on count %d\n", count);

				}
				printf("This is i variable %d\n", i);
				//printf("This is total count before the increment %d\n", total_count);
				if(count >= 2){
					printf("This is total count before the increment %d\n", total_count);
					printf("count is more than two so increment total_count\n");
					total_count++;
					printf("This is total count after increment %d\n", total_count);
				}
			}
			printf("This is count before it resets to 0 %d\n", count);
			printf("Number of connections for noded %d based on count %d\n", array[row]->id, count);
			printf("THIS IS NODE COUNT %d AND TOTAL_COUNT %d\n", *node_count, total_count);
			if(count == 1){
				printf("count == 1\n");
				return_val = -2;
				break;
			}else if(total_count == *node_count){
				printf("DEBUG STATEMENT\n");
				//return_val = 0;
				node_removal = 0;
				break;
			}
		}

		printf("This is node removal else statement\n return value equals %d\n", return_val);
		if(return_val != -1 && return_val != -2){
			printf("All Zergs in position.\n");
			node_removal = 0;
			//return_val = 1;
			break;
		}else if(return_val == -1){
			node_removal = 0;
			break;
		}
	}

	printf("This is return value before return %d\n", return_val);
	return(return_val);
}
*/
int which_node_to_remove(int *node_count, BST *nodes[], int **matrix)
{
	int neighbor;
	int return_val = 0;
	for(int row = 0; row < *node_count; row++){
		neighbor = 0;
		for(int column = 0; column < *node_count; column++){
			if(matrix[row+1][column+1] == 1){
				neighbor++;
			}
		}
		if(neighbor == 1 && return_val != -1){
			return_val = remove_node(node_count, nodes, row, matrix);
		}
	}
	return(return_val);
}

int remove_node(int *node_count, BST *nodes[], int row, int **matrix)
{
	static double total_removal = 0;
	total_removal++;
	int return_val;
	if((total_removal / *node_count) < .50){
		//total_removal++;
		node_neighbor_removal(row + 1, node_count, matrix);
		nodes[row]->connected = -1;
		nodes[row]->removed = -1;
		return_val = -2;
	}else{
		//evaluate if statement still made
		//printf("To many nodes to remove. Might want to add nodes instead\n");
		return_val = -1;
	}
	return(return_val);
}

int remove_smallest_set(int *set_array, int set_pos, BST *node[], int *node_count, int **matrix)
{
	int count = 0;
	double largest_set_size = 0;
	for(int i = 0; i < *node_count; i++){
		if(set_pos == set_array[i]){
			largest_set_size++;
		}
	}
	//printf("The largest set size is %lf\n", largest_set_size);
	//printf("This is node_count %d\n", *node_count);
	//printf("This is largest set size divided by node_count %lf\n", largest_set_size / *node_count);
	if((largest_set_size / *node_count) > .50 && (largest_set_size / *node_count > 1.00 || largest_set_size / *node_count < 1.00)){
		printf("To many nodes to remove based on reachability may want to add nodes instead\n");
		return(-1); // 0 indicates nothing removed
	}
	else if((int)largest_set_size == *node_count){
		return(1);
	}
	for(int i = 0; i < *node_count; i++){
		if(set_array[i] != set_pos){
			node[i]->connected = -1;
			node[i]->removed = -1;
			node[i]->removed_from_set = 1;
			node_neighbor_removal(i+1, node_count, matrix);
			count = 1; // a node was removed
		}
	}
	// 0 means no nodes removed so set is good
	return(count);
}

void node_neighbor_removal(int row, int *node_count, int **matrix)
{
	for(int column = 0; column < *node_count; column++){
		matrix[row][column+1] = -1;
		matrix[column+1][row] = -1;
	}

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
	latitude1 *= M_PI/180;
	latitude2 *= M_PI/180;
	longitude1 *= M_PI/180;
	longitude2 *= M_PI/180;



	lat_change = (latitude2 - latitude1) / 2;
	printf("This is lat_change %lf\n", lat_change);
	lat_change = (sin(lat_change) * sin(lat_change));
	printf("Sin squared of lat_change %lf\n", lat_change);
	
	//lat_change = sin(lat_change/2) * sin(lat_change/2);

	double long_change;
	long_change = (longitude2 - longitude1) / 2;
	printf("This is long_change %lf\n", long_change);
	//long_change = sin(long_change/2) * sin(long_change/2);
	long_change = (sin(long_change) * sin(long_change));
	printf("sin squared of long_change %lf\n", long_change);
	double a;
	a = lat_change + (cos(latitude1) * cos(latitude2) * long_change);
	printf("This is value of a: %lf", a);
	double c;
	c = 2 * atan2(sqrt(a), sqrt(1-a));
	printf("This is value of c %lf\n", c);
	
	distance = radius * c;
	printf("This is distance before conversion %lf\n", distance);
	//inside_radical = lat_change + (cos(latitude1) * cos(latitude2) * long_change);
	//distance = (2 * radius) * asin(sqrt(inside_radical));

	double altitude_change;
	altitude_change = fabs(altitude2 - altitude1);
	//altitude_change = altitude_change * ((6 * 12)/39.37);
	altitude_change *= 1.8288; //try this conversion
	double true_distance; 
	true_distance = sqrt((distance * distance) + (altitude_change * altitude_change));
	return true_distance;

}

void inorder(BST *root, int *count)
{
	if(root != NULL){
		inorder(root->left, count);
		(*count) += 1;
		//printf("ID %d", root->id);
		//if(fabs(root->latitude) <= 90){
		//	printf(" lat %lf", root->latitude);
		//}
		//if(fabs(root->longitude) <= 180){
		//	printf(" long %lf", root->longitude);
		//}
		//if(root->hp > 0){
		//	printf(" maxhp %d", root->maxhp);
		//}
		//if(root->maxhp > 0){
		//	printf(" hp %d", root->hp);
		//}else{
		//	printf("\n");
		//}
		//printf("\n");

		
		inorder(root->right, count);
	}
}

void tree_insert(struct BinarySearchTree *root, double longitude, double altitude, double latitude, int id, int hp, int maxhp, int type)
{
	if(id < root->id){
		if(root->left != NULL){
			tree_insert(root->left, longitude, altitude, latitude, id, hp, maxhp, type);
		}else{
			root->left = init_tree(longitude, altitude, latitude, id, hp, maxhp);

		}
	}
	else if(id > root->id){
		if(root->right != NULL){
			tree_insert(root->right, longitude, altitude, latitude, id, hp, maxhp, type);
		}else{
			root->right = init_tree(longitude, altitude, latitude, id, hp, maxhp);
		}
	}else{
		//printf("root->id %d and zergSourceID %d should be the same\n", root->id, id);
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
	printf("This is node hp %d\n", hp);
	t->maxhp = maxhp;
	t->removed = 0;
	t->removed_from_set = 0;
	t->connected = -5;
	t->globalPosition = '1';
	return(t);
}

long unsigned swap32(long unsigned val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}


/*char *zergBreed(int breedType)
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
	//if need to revert delete the htonl one and uncomment all these
	uint8_t sign;
	uint16_t exponent;
	uint64_t mantissa;
	double result = 0;
	int neg_or_pos;

	sign = num >> 63;
	if(sign == 1){
		neg_or_pos = -1;
	}else{
		neg_or_pos = 1;
	}
	printf("Sign of number is %d\n", sign);
	exponent = (num >> 52 & 0x7FF) - 1023;
	mantissa = num & 0xFFFFFFFFFFFFF;
	result = (mantissa *pow(2, -52)) + 1;
	result *= pow(1, sign) * pow(2, exponent);
	return(result * neg_or_pos);
	// Try below instead to see if can get negative values
	//uint32_t high_part = htonl((uint32_t)(num >> 32));
	//uint32_t low_part = htonl(((uint32_t)(num & 0xFFFFFFFFLL)));
	//mantissa = (((uint64_t)low_part) << 32) | high_part;
	//result = (double)mantissa;
	//return(result);

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


/////////////////////////

//int *set_array = malloc(*my_count * sizeof(int));//for(int i = 0
void make_set_array(int *set_array, int *node_count, int **matrix, BST *nodes[])
{
	for(int i = 0; i < *node_count; i++){
		if((fabs(nodes[i]->latitude) > 90) || (fabs(nodes[i]->longitude) > 180) || (fabs(nodes[i]->altitude) > 6160)){
				nodes[i]->globalPosition = '0';
		}else{
			nodes[i]->globalPosition = '1';
		}
	}
	int count = 0;
	for(int i = 0; i < *node_count; i++){
		if(nodes[i]->globalPosition == '0'){
			count++;
		}
	}
	if(count == *node_count){
		//print_hp(hp_threshold, nodes, node_count);
		printf("None of the nodes passed were passed with proper gps data. Can't graph it. program will exit.\n");
		exit(1);
	}
	for(int i = 0; i < *node_count; i++){
		set_array[i] = i;
		//printf("%d\n", set_array[i]);
	}
	// go through set and update for reachability based on array connections.
	for(int i = 0; i < *node_count; i++){
		for(int j = 0; j < *node_count; j++){
			if((i + 1) == (j + 1)){
				continue;
			}else{
				if(matrix[i+1][j+1] == 1 && nodes[i]-> globalPosition == '1'){
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
					if(nodes[i]->globalPosition == '0'){
						set_array[i] = -1;
					}
					continue;
				}
			}
		}
	}
}

