////////////////////////////////////////////
//                                        //
//        LIRS replacement policy         //
//     Benji Cruz, benjicruz@g.ucla.edu   //
//                                        //
////////////////////////////////////////////

#include "../inc/champsim_crc2.h"

#define NUM_CORE 1
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16
#define MAX_LIRS_STACK 9 // (LLC_WAYS + 1);

struct Block {
    bool valid;
    bool high_low; // true is high inter reference, false low interference 
};

Block lirs_blocks[LLC_SETS][LLC_WAYS];
uint32_t lirs_stack[LLC_SETS][MAX_LIRS_STACK];

// initialize replacement state
void InitReplacementState()
{
    cout << "Initialize LIRS replacement state" << endl;
    for (int i = 0; i < LLC_SETS; i++) {
        for (int j = 0; j < LLC_WAYS; j++) {
            lirs_blocks[i][j].valid = false;
            lirs_blocks[i][j].high_low = false;
        }
        for (int k = 0; k < MAX_LIRS_STACK; k++) {
            lirs_stack[i][k] = UINT32_MAX; // Initialize stack with invalid indices
        }
    }
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    for (int i = 0; i < MAX_LIRS_STACK; i++) {
        if (lirs_stack[set][i] != UINT32_MAX && !lirs_blocks[set][lirs_stack[set][i]].high_low) {
            return lirs_stack[set][i];
        }
    }
    for (int i = 0; i < LLC_WAYS; i++) {
        if (!lirs_blocks[set][i].high_low) {
            return i;
        }
    }
    return lirs_stack[set][MAX_LIRS_STACK - 1];
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    if (hit) {
        for (int i = 0; i < MAX_LIRS_STACK; i++) {
            if (lirs_stack[set][i] == way) {
                for (int j = i; j > 0; j--) {
                    lirs_stack[set][j] = lirs_stack[set][j - 1];
                }
                lirs_stack[set][0] = way;
                break;
            }
        }
    } else {
        uint32_t victim_way = GetVictimInSet(cpu, set, nullptr, PC, paddr, type);
        lirs_blocks[set][victim_way].valid = true;
        lirs_blocks[set][victim_way].high_low = true;
        for (int i = MAX_LIRS_STACK - 1; i > 0; i--) {
            lirs_stack[set][i] = lirs_stack[set][i - 1];
        }
        lirs_stack[set][0] = victim_way;
    }
}
  

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat(){}

// use this function to print out your own stats at the end of simulation
void PrintStats(){}
