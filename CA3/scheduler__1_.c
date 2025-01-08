#include <stdio.h>
#include "utlist.h"
#include "utils.h"
#include "memory_controller.h"

extern long long int CYCLE_VAL;

int drain_writes[MAX_NUM_CHANNELS]; // Keep track of the write and drain for the channels

#define MAX_NUM_RANKS 8 //Tracking
#define MAX_NUM_BANKS 16 //Tracking 

int get_open_row[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS]; // used to get the currently open row in a bank
int row_open(int channel, int rank, int bank); // Prioritize memory requests that access the open row


void init_scheduler_vars()
{
    for (int i = 0; i < MAX_NUM_CHANNELS; i++) {
        drain_writes[i] = 0; //Standard
        for (int j = 0; j < MAX_NUM_RANKS; j++) {
            for (int k = 0; k < MAX_NUM_BANKS; k++) {
                get_open_row[i][j][k] = -1; //Initialize with a default of no row being open. 
            }
        }
    }
    return;
}

#define HI_WM 80 //Changed the thresholds to increase accumulation and clearing for potentially more throughput
#define LO_WM 50

void schedule(int channel)
{
    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;

    // if in write drain mode, keep draining writes until the
	// write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; //We shall drain
    } else {
        drain_writes[channel] = 0; //We shall not drain
    }

	// initiate write drain if either the write queue occupancy
	// has reached the HI_WM , OR, if there are no pending read
	// requests
    if(write_queue_length[channel] > HI_WM) {
        drain_writes[channel] = 1; 
    } else {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1; 
    }


    // If in write drain mode, look through all the write queue
	// elements (already arranged in the order of arrival), and
	// issue the command for the first request that is ready
    if(drain_writes[channel]) {
        LL_FOREACH(write_queue_head[channel], wr_ptr) {
            if(wr_ptr->command_issuable) {
                issue_request_command(wr_ptr); // IsQsue the first issuable write request
                break;
            }
        }
        return;
    }

    request_t * best_rd_ptr = NULL; // Start implimentdation of RLDP Prioritize read requests that access the open row

    LL_FOREACH(read_queue_head[channel], rd_ptr) {
        if (rd_ptr->command_issuable) {
            if (best_rd_ptr == NULL || 
                (rd_ptr->dram_addr.row == row_open(rd_ptr->dram_addr.channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank))) {
                best_rd_ptr = rd_ptr; /// If this read request hits the open row, prioritize it
            }
        }
    }

    if (best_rd_ptr) { //Here we are issueing the request command based on the 'prioritzed' one (open row)
        issue_request_command(best_rd_ptr);
        return;
    }else{
    LL_FOREACH(read_queue_head[channel], rd_ptr) { // If no row-hit requests, go back to FCFS as default.
        if (rd_ptr->command_issuable) {
            issue_request_command(rd_ptr);
            break;
        }
    }
    }
}

void scheduler_stats()
{
    /* No statistics to print for now. */
}

int row_open(int channel, int rank, int bank) { // Afformentioned function 
    return get_open_row[channel][rank][bank];
}
