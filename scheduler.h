#pragma once
#include "Router.h"

////////////////////////
///// PROTOTYPES ///////
////////////////////////

int send_packet_RR(flow** flow_array, int* turn, int current_time, packet** nextpacket);
int send_packet_DRR(flow **flow_array, int *turn, int current_time, packet** nextpacket);
int give_credits(flow **flow_array);