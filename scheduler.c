# include "scheduler.h"

///////////////////////
////// FUNCTIONS //////
///////////////////////

int send_packet_RR(flow** flow_array, int* turn, int current_time, packet** nextpacket)
{
	int time_added = 0;
	char output_line[MAX_LINE_LEN];

	for (int current_index = (*turn) + 1; current_index < MAX_FLOWS; current_index++)
	{
		if (flow_array[current_index] == NULL)
		{
			current_index = 0;
			continue;
		}
		if (flow_array[current_index]->head != NULL)
		{
			*turn = current_index;
			for (int i = 0; i < flow_array[current_index]->weight; i++)
			{
				if (flow_array[current_index]->head != NULL)
				{
					time_added += (flow_array[current_index]->head)->length;
					snprintf(output_line, MAX_LINE_LEN, "%d: %d", current_time + time_added, (flow_array[current_index]->head)->packetID);
					fputs(output_line, fp_out);
					update_flow_array(flow_array, nextpacket);
					(flow_array[current_index]->head) = (flow_array[current_index]->head)->next;
				}
				else
				{
					break;
				}
			}
			return time_added;

		}
		if (current_index == *turn) // Indicates we have itterated over all flows and sent nothing
		{
			return 1;
		}
		if (current_index == MAX_FLOWS - 1) // When reach the last flow possible go back to the first flow 
		{
			current_index = 0;
		}
	}
}

int send_packet_DRR(flow **flow_array, int *turn, int current_time, packet** nextpacket)
{
	int flow_index = *turn + 1;
	flow *cur_flow = NULL;
	packet *potential_packet = NULL;
	int service_time = 0;

	// If no flows have packets that need to be sent
	if (flow_array_empty(flow_array))
	{
		*turn = 0;
		return 1; 
	}

	// Loop intended to find the flow
	while (1)
	{
		// Init current flow
		cur_flow = flow_array[flow_index];

		// If reached end of flow_array, give out credits and start iterating again
		if (!cur_flow)
		{
			give_credits(flow_array);
			flow_index = 0;
			continue;
		}

		// Flow isnt NULL, init packet reached
		potential_packet = cur_flow->head;
		
		// Found sendable flow
		if (NULL != potential_packet && (cur_flow->credits > potential_packet->length))
		{
			*turn = flow_index;
			break;
		}

		flow_index++;
	}

	// At this point, the next flow to be served has been found. Now serve flow
	while (NULL != potential_packet)
	{
		// Send current packet
		service_time += potential_packet->length;
		fprintf(fp_out, "%d: %d\n", current_time + service_time, potential_packet->packetID);
		cur_flow->credits -= potential_packet->length;
		update_flow_array(flow_array, nextpacket);

		// Update flow and next packet to be sent
		cur_flow->head = potential_packet->next;
		potential_packet = cur_flow->head;

		//Check if there are enough credits to send another packet
		if (NULL != potential_packet && cur_flow->credits < potential_packet->length)
			break;	
	}

	return service_time;
}

int give_credits(flow **flow_array)
{
	int flow_index = 0;

	while (1)
	{
		// Reached end of array
		if (!flow_array[flow_index])
			return 1;

		// Else
		flow_array[flow_index]->credits += quantum * flow_array[flow_index]->weight;
		flow_index++;
	}
}