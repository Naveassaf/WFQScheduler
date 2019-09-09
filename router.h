#pragma once
#pragma warning(disable:4996)

////////////////////////
////// LIBRARIES ///////
////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////
////// CONSTANTS ///////
////////////////////////

#define MAX_LINE_LEN 200
#define DONE_READING_FILE 444
#define STILL_READING_FILE 555
#define SUCCESSFUL_SCHEDULING 1
#define FAILED_SCHEDULING 0
#define DRR 13
#define RR 12
#define IP_MAX_LEN 50
#define MAX_FLOWS 32000

////////////////////////
/////// GLOBALS ////////
////////////////////////

int quantum;
FILE* fp_out;
FILE* fp_in;

////////////////////////
////// STRUCTURES //////
////////////////////////

typedef struct _packet_ {
	long int packetID;
	long int time;
	char Sadd[IP_MAX_LEN];
	int Sport;
	char Dadd[IP_MAX_LEN];
	int Dport;
	int length;
	struct _packet_ *next;
}packet;

typedef struct _flow_ {
	packet* head;
	packet* last;
	int credits;
	int weight;
	char Sadd[IP_MAX_LEN];
	int Sport;
	char Dadd[IP_MAX_LEN];
	int Dport;
}flow;

////////////////////////
///// PROTOTYPES ///////
////////////////////////

packet* create_packet(char *line, int *flow_weight);
int update_flow_array();
int packet_from_flow(flow cur_flow, packet cur_packet);
int initialize_flow_array(flow **flow_array);
int flow_array_empty(flow **flow_array);
int main(int argc, char **argv);
int schedule(flow** flow_array);