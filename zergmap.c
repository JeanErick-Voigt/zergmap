#include <netinet/in.h>
#include <byteswap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stddef.h>
#include <math.h>
#include "zergmap_structs.h"

#define NTOH2(x) (((x << 8) & 65280) + (x >> 8))
#define NTOH3(x) ((int) x[0] << 16) | ((int) (x[1]) << 8) | ((int) (x[2]))
#define NTOH4(x) ((int) x[0] << 24) | ((int) (x[1]) << 16) | ((int) (x[2]) <<  8) | ((int) (x[3]))

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
	double val;
	double hp_threshold = -1.00;
	while((option = getopt(argc, argv, "h:")) != -1){
		switch(option){
			case 'h':
				val = (double)atoi(optarg);
				for(int i = 0; i < (int)strlen(optarg); i++){
					if(optarg[i] < 47 || optarg[i] > 57){
						printf("%s is improper HP flag entry option for -h because of non integer character(s) or negative number."
								" Program will exit\n", optarg);
						exit(1);
					}
				}
				hp_threshold = (double)(val/100);
				break;
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
					fread(&status, sizeof(status), 1, fp1);
					fread(messagePayload, messageLength, 1, fp1);
					int statusType = status.statusType; 
					uint32_t  speed = NTOH4(status.speed);
					hitpoints = NTOH3(status.hitPoints);
					maxhp = NTOH3(status.maxHitPoints);
					double nSpeed = convert32ToDouble(speed);
					break;

				case 3:
					fread(&gpsDataPayload, sizeof(gpsDataPayload), 1, fp1);
					uint64_t lat = ntoh64(gpsDataPayload.latitude);
					newLat = (double)convert64ToDouble(lat);
					uint64_t longitude = ntoh64(gpsDataPayload.longitude);
					newLong = (double)convert64ToDouble(longitude);

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
	printf("Below health (%lf%%)\n", (hp_threshold * 100));
	//for(int i = 0; i < *my_count; i++){
	//	printf("This is node %d,  hp %d nodes maxhp %d and hp_threshold %lf\n", nodes[i]->id, nodes[i]->hp, nodes[i]->maxhp,
	//		hp_threshold);
	//	if(nodes[i]-> hp < (hp_threshold * nodes[i]->maxhp) && nodes[i]->hp >= 0){
	//		printf("REMOVED ZERG #%d, hp: %d, maxhp: %d\n", nodes[i]->id, nodes[i]->hp, nodes[i]->maxhp);
	//	}
	//}
	//printf("printing hp below debug debug\n\n");
	print_hp(hp_threshold, nodes, my_count);

	double distance;
	int **adjacency = malloc((*my_count + 1) * sizeof(int *));
	int **temp_adjacency = malloc((*my_count + 1) * sizeof(int *));
	for(int i = 0; i < *my_count + 1; i++){
		adjacency[i] = (int *)malloc((*my_count + 2) * sizeof(int));
		temp_adjacency[i] = (int *)malloc((*my_count + 2) * sizeof(int));
	}
	for(int i = 0; i < *my_count + 1; i++){
		for(int j = 0;  j < *my_count + 2; j++){
			if(i == 0 && j == 0){
				adjacency[0][0] = -5;
			}
			else if(i == 0 && j != 0){
				if(j < *my_count + 1 && fabs(nodes[j-1]->altitude) <= 6160 && fabs(nodes[j-1]->latitude) <= 90
				    && fabs(nodes[j-1]->longitude) <= 180){
					adjacency[i][j] = nodes[j-1]->id;
				}else{
					adjacency[i][j] = -5;
				}
			}
			else if(i != 0 && j == 0){
				if(fabs(nodes[i-1]->altitude) <= 6160 && fabs(nodes[i-1]->latitude) <= 90
				     && fabs(nodes[i-1]->longitude) <= 180){
					adjacency[i][j] = nodes[i-1]->id;
				}else{
					adjacency[i][j] = -5;
				}
			}
			else if(j == *my_count + 1){
				adjacency[i][j] = -5;
			}else{
				distance = haversine_formula(nodes[i-1]->longitude, nodes[i-1]->latitude, nodes[i-1]->altitude,
					nodes[j-1]->longitude, nodes[j-1]->latitude, nodes[j-1]->altitude);
				if(distance >= (1.25000 * .9144) && distance <= 15.00000){
					adjacency[i][j] = 1;
				}else{
					adjacency[i][j] = 0;
				}
			}
		}
		printf("\n");
	} 

//	print_matrix(adjacency, my_count);

	int removed, neighbors;
	int *set_array = malloc(*my_count * sizeof(int));//for(int i = 0
	print_hp(hp_threshold, nodes, my_count);
	make_set_array(set_array, my_count, adjacency, nodes);
	int set_pos;
	set_pos = find_largest_set(set_array, my_count); //this returns set position
	removed = remove_smallest_set(set_array, set_pos, nodes, my_count, adjacency);
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

	if(fp1 != NULL){
		fclose(fp1);
	}
	return(0);
}

/*int distinct_paths(int *array, int *node_count, int **matrix, BST *nodes[])
{
	int blackout_index = 0;
	int set_pos;
	int *new_array = *array;
	int count = 0;
	for(int i = 0; i < *node_count; i++){
		if(nodes[i]->removed_from_set == 1){
		new_array[i] = -1;
		}else{
			new_array[i] = i;
			count = 0;
		}
	}
	int changed;
	while(blackout < *node_count){
		blackout_index++;
		changed = 0;
		for(int compare_a = 0; compare_a < *node_count;){
			for(int compare_b = 0; compare_b < *node_count; compare_b++){
				if(matrix[a+1][b+1] == 1 && (new_array[a] != blackout_index && new_array[b] != blackout_index)
					changed++;
					if(array[a] < array[b]{
						new_array[b] = new_array[a];
					}else{
						new_array[a] = new_array[b];
					}
				}else{
					continue;
				}
			}
		
		}
		set_pos = find_largest_set(new_array, node_count);
		int array_count
		if(changed > 0){
			for(int i = 0; i < *node_count; i++){
				if(new_array[i] == set_pos){
					array_count++;
			}
			if(array_count = count);
				//blackout = *node_count + 1;
				continue;
			}else{
				remove_smallest_set(new_array, set_pos, nodes, node_count, matrix);
				*set_array = *new_array;
			}
		}
	}
	
}
*/
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

	int node_removal = 1;
	int good_row;
	int neighbor_count = 0;
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
	lat_change = (sin(lat_change) * sin(lat_change));
	

	double long_change;
	long_change = (longitude2 - longitude1) / 2;
	long_change = (sin(long_change) * sin(long_change));
	double a;
	a = lat_change + (cos(latitude1) * cos(latitude2) * long_change);
	double c;
	c = 2 * atan2(sqrt(a), sqrt(1-a));
	
	distance = radius * c;

	double altitude_change;
	altitude_change = fabs(altitude2 - altitude1);
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

