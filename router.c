#include "scheduler.h"

////////////////////////
/// GLOBAL VARIABLES ///
////////////////////////

int scheduling_algo;
int default_weight;

///////////////////////
////// FUNCTIONS //////
///////////////////////

int update_flow_array(flow **flow_array, long int current_time, packet** next_packet)
{
	int flow_index = 0;
	char cur_line[MAX_LINE_LEN];
	packet* new_packet = NULL;
	flow* new_flow = NULL;
	int cur_flow_weight = -1;
	int packet_added = 0;

	// If first iteration, read first line in file as first packet
	if (NULL == (*next_packet))
	{
		if (NULL == fgets(cur_line, MAX_LINE_LEN, fp_in))
			return DONE_READING_FILE;
		new_packet = create_packet(cur_line, &cur_flow_weight);
		*next_packet = new_packet;
	}
	else
	{
		new_packet = *next_packet;
	}


	// Deals with case in which next packet "has not arrived yet"
	if (new_packet->time > current_time)
		return STILL_READING_FILE;

	// Start appending packets to flow_array until reaching a packet whose time is greater than current time
	while (1)
	{
		// If passed last flow without finding match, enter new flow into array
		if (NULL == flow_array[flow_index])
		{
			// Allocate room for new flow and initialize its values
			if (NULL == (new_flow = (flow*)malloc(sizeof(flow))))
			{
				fprintf(stderr, "Memory allocation failed!\n");
				return FAILED_SCHEDULING;
			}
			new_flow->head = new_packet;
			new_flow->last = new_packet;
			new_flow->Sport = new_packet->Sport;
			new_flow->Dport = new_packet->Dport;
			strncpy(new_flow->Sadd, new_packet->Sadd, strlen(new_packet->Sadd)+1);
			strncpy(new_flow->Dadd, new_packet->Dadd, strlen(new_packet->Dadd)+1);
			new_flow->credits = 0;
			// No flow weight provided
			if(-1 == cur_flow_weight)
				new_flow->weight = default_weight;
			else
				new_flow->weight = cur_flow_weight;
			// "Insert" newly created flow into array
			flow_array[flow_index] = new_flow;
			packet_added = 1;
		}
		// If the current flow we indexed matches the current packet made from the input file
		else if (packet_from_flow(*flow_array[flow_index], *new_packet))
		{
			//If flow currently has no queueing packets
			if (!flow_array[flow_index]->last)
			{
				flow_array[flow_index]->head = new_packet;
				flow_array[flow_index]->last = new_packet;

			}
			else
			{
				flow_array[flow_index]->last->next = new_packet;
				flow_array[flow_index]->last = new_packet;
			}
			packet_added = 1;
		}
		else
		{
			flow_index++;
		}

		// if added a packet to flow array, read in next packet from input file and if relevant, add to flow array
		if (packet_added)
		{
			if (NULL == fgets(cur_line, MAX_LINE_LEN, fp_in))
				return DONE_READING_FILE;
			cur_flow_weight = -1;
			new_packet = create_packet(cur_line, &cur_flow_weight);
			*next_packet = new_packet;
			flow_index = 0;
			packet_added = 0;
			if (new_packet->time > current_time)
				break;
		}
	}

	// Update next packet to the last packet read in
	*next_packet = new_packet;

	return STILL_READING_FILE;
}

int packet_from_flow(flow cur_flow, packet cur_packet)
{
	if (0 != strcmp(cur_flow.Sadd, cur_packet.Sadd))
		return 0;
	if (0 != strcmp(cur_flow.Dadd, cur_packet.Dadd))
		return 0;
	if (cur_flow.Dport != cur_packet.Dport || cur_flow.Sport != cur_packet.Sport)
		return 0;
	return 1;
}

packet* create_packet(char *line, int *flow_weight)
{
	packet *new_packet = NULL;
	int spaces_passed = 0, at_space = 0;
	int start_index = 0, cur_index, data_len = 0;
	char cur_data[MAX_LINE_LEN];

	// Allocate memory for packet
	if (NULL == (new_packet = (packet*)malloc(sizeof(packet))))
	{
		fprintf(stderr, "Memory allocation failed!");
		return new_packet;
	}

	// Parse line into packet parameters.
	for (cur_index = 0; cur_index <= strlen(line); cur_index++)
	{
		// reached end of current data parameter (space, newline or end of str)
		if (line[cur_index] == ' ' || line[cur_index] == '\n' || line[cur_index] == '\0')
		{
			data_len = cur_index - start_index;
			strncpy(cur_data, line+start_index, data_len);
			at_space = 1;
		}

		if (at_space)
		{
			
			//based on number of spaces passed, save cur_data into new_packet
			switch (spaces_passed)
			{
			case 0:	new_packet->packetID = atoi(cur_data); break;
			case 1:	new_packet->time = atoi(cur_data); break;
			case 2: strncpy(new_packet->Sadd, cur_data, data_len);
					new_packet->Sadd[data_len] = '\0'; break;
			case 3: new_packet->Sport = atoi(cur_data); break;
			case 4: strncpy(new_packet->Dadd, cur_data, data_len); 
					new_packet->Dadd[data_len] = '\0'; break;
			case 5: new_packet->Dport = atoi(cur_data); break;
			case 6: new_packet->length = atoi(cur_data); break;
			case 7: *flow_weight = atoi(cur_data); break;
			}

			spaces_passed++;
			start_index = cur_index + 1;
			at_space = 0;
			// Clean temporary data string (cur_data)
			for (int i = 0; i < data_len; i++)
				cur_data[i] = '\0';
		}

		// reached end of line for this packet
		if (line[cur_index] == '\n' || line[cur_index] == '\0')
			break;

	}

	new_packet->next = NULL;

	return new_packet;
}

int initialize_flow_array(flow **flow_array)
{
	for (int i = 0; i < MAX_FLOWS; i++)
		flow_array[i] = NULL;
}

int flow_array_empty(flow **flow_array)
{
	for (int i = 0; i < MAX_FLOWS; i++)
		if (NULL != flow_array[i] && NULL != flow_array[i]->head)
			return 0;
	return 1;
}

int schedule(flow** flow_array)
{
	packet *next_packet = NULL;
	int current_time = 0;
	int turn = -1;
	int status = STILL_READING_FILE;

	while (1)
	{
		if(scheduling_algo == RR)
			current_time += send_packet_RR(flow_array, &turn, current_time, &next_packet);
		else
			current_time += send_packet_DRR(flow_array, &turn, current_time, &next_packet);

		if (status == DONE_READING_FILE) // We need to also check that all packets were sent already
		{
			if (flow_array_empty(flow_array))
			{
				break;
			}
			continue;
		}
		else
		{
			status = update_flow_array(flow_array, current_time, &next_packet);
		}

	}
	return SUCCESSFUL_SCHEDULING;
}

int main(int argc, char **argv)
{
	flow *flow_array[MAX_FLOWS];

	// Read in command line arguments. Example cmd line call: sch.exe DRR input_file.txt output_file.txt 10 64
	if (argc != 6)
	{
		fprintf(stderr, "Incorrect number of parameters provided!\n");
		return FAILED_SCHEDULING;
	}

	// Initialize flow array to NULLs
	if (!initialize_flow_array(flow_array))
		return FAILED_SCHEDULING;

	// Parse scheduling algo parameter
	if (0 == strcmp(argv[1], "DRR"))
		scheduling_algo = DRR;
	else
		scheduling_algo = RR;

	// Open input and output files provided
	fp_in = fopen(argv[2], "r");
	if (fp_in == NULL)
		printf("Error opening input file\n");

	fp_out = fopen(argv[3], "w");
	if (fp_out == NULL)
		printf("Error opening input file\n");

	// Parse default weight
	default_weight = atoi(argv[4]);

	// Parse quantum (o incase of RR)
	quantum = atoi(argv[5]);
	
	schedule(flow_array);

	// Close used resources and exit with "success" exit code
	fclose(fp_in);
	fclose(fp_out);

	return SUCCESSFUL_SCHEDULING;
}