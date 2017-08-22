#include <stdint.h>


typedef struct PcapFileHeader {
	uint32_t magicNumber;  // File_Type_ID
	uint16_t majorVersion; // Major_Version
	uint16_t minorVersion;  // Minor_Version
	uint32_t thisZone;  // GMT Offset
	uint32_t timestampAcc;  // Accuracy Delta
	uint32_t captureLength; // Maximum Length of a Capture
	uint32_t linklayerType;   //Link Layer Type


} TOPHEADER; 

typedef struct PcapPacketHeader {
	uint32_t unixEpoch;
	uint32_t usFromEpoch;
	uint32_t lengthDataCaptured;
	uint32_t untruncatedPacketLength;
} PACKETHEADER;

typedef struct EthernetFrame {
	uint16_t destMac[3];
	uint16_t sMac[3];
	uint16_t ethernetType; 
	
} ETHERNET;

typedef struct ipv4Header {
	uint8_t versionAndIHL;
	uint8_t dscpAndECN;
	uint16_t iptotalLength;
	uint16_t Identification;
	uint16_t flagstoFragmentoffset;
	uint16_t ttltoProtocol;
	uint16_t headerChecksum;
	uint32_t sourceIP;
	uint32_t destIP;
} IPHEADER;

typedef struct udpHeader {
	uint16_t sourcePort;
	uint16_t destPort;
	uint16_t Length; 
	uint16_t checksum;
}UDP;

typedef struct zergPacketHeader {
	uint8_t versionToType;
	uint8_t totalLength[3];
	uint16_t sourceZergID;
	uint16_t destinationZergID;
	//uint32_t sequenceID;  try to do it in an array instead conversino
	uint8_t sequenceID[4];
}ZERG;

typedef struct statusPayload {
	uint8_t hitPoints[3];
	uint8_t armor;
	uint8_t maxHitPoints[3];
	uint8_t statusType;
	uint8_t speed[4];
}STATUSPAYLOAD;

typedef struct commandPayload {
	uint16_t commandField;
} COMMAND;

typedef struct gpsDataPayload{
	uint8_t longitude[8];
	uint8_t latitude[8];
	uint8_t altitude[4];
	uint8_t bearing[4];
	uint8_t speed[4];
	uint8_t accuracy[4];
}GPS;


typedef struct BinarySearchTree{
	double altitude;
	double latitude;
	double longitude;
	int id;
	int hp;
	int maxhp;
	struct BinarySearchTree *left;
	struct BinarySearchTree *right;
}BST;
