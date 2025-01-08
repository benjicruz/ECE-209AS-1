# -*- coding: utf-8 -*-
"""Cache Replacement Tournament

Automatically generated by Colab.

Original file is located at
    https://colab.research.google.com/drive/1VO6JCPpQSqHKl8ImTXbbVDYZfnLnv9gl
"""

#include "../inc/champsim_crc2.h"
#include <iostream>
#include <cstdlib> // For rand()


#define NUM_CORE 1
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16
#define epsilon 0.1 //Used as the randomization threshold for BIP

//Defining all the caches for the hybrid replacement policy
uint32_t lru[LLC_SETS][LLC_WAYS]; //LRU Cache
uint32_t bip[LLC_SETS][LLC_WAYS]; //BIP Cache
uint32_t winner[LLC_SETS][LLC_WAYS]; //Winner's cache, will be used for the Hybrid Policy


// Auxiliary Tag Directory (ATD) and Saturating Counter (SCTR) - Used to try and create a tournament system. Always yeilded worse results :(
bool atd_x[LLC_SETS]; //Tag for policy X
bool atd_y[LLC_SETS];  //Tag for policy y
int sctr[LLC_SETS]; //Saturating Counter


// Initialize replacement state for all caches
void InitReplacementState()
{
   cout << "Initialize LRU and BIP replacement states" << endl;


   for (int i = 0; i < LLC_SETS; i++) {
   	for (int j = 0; j < LLC_WAYS; j++) {
       	lru[i][j] = j; //LRU Cache
       	bip[i][j] = j; //BIP Cache
       	winner[i][j] = j; //WINNER Cache
   	}
   }


   // Initialize Saturating Counters and ATDs
   for (int i = 0; i < LLC_SETS; i++) {
   	atd_x[i] = false;
   	atd_y[i] = false;
   	sctr[i] = 0; //Saturating Counter
   }
}


//We will want to find the vitcim we will replace in our winner cache.
uint32_t GetVictimInSet(uint32_t cpu, uint32_t set, const BLOCK* current_set, uint64_t PC, uint64_t padAdr, uint32_t type)
{
   for (int i = 0; i < LLC_WAYS; i++) {
   	if (winner[set][i] == (LLC_WAYS - 1)) {
       	return i;
   	}
   }
   return 0;
}


//Update the replacement state for our LRU
void UpdateLRUReplacementState(uint32_t set, uint32_t way)
{
   for (uint32_t i = 0; i < LLC_WAYS; i++) {
   	if (lru[set][i] < lru[set][way]) {
       	lru[set][i]++;
   	}
   }
   lru[set][way] = 0; //In LRU we should only be promoting to MRU on hit to make LIP, keeping this method performs best so unchanged.
}


// Create a BIP function alongside to handle between thrashing and spatial locality
void UpdateBIPReplacementState(uint32_t set, uint32_t way)
{
   for (uint32_t i = 0; i < LLC_WAYS; i++) {
   	if (bip[set][i] < bip[set][way]) {
       	bip[set][i]++;
   	}
   }
   if (rand() < epsilon * RAND_MAX) { //Under a certain threshold. In BIP we are mostly inserting at LRU at the moment.
   	bip[set][way] = 0; // Insert at MRU position
   } else {
   	bip[set][way] = LLC_WAYS - 1; // Insert at LRU position
   }
}


// Call this at every cache hit essentially.
void UpdateReplacementState(uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
   // Used to update our LRU replacement state
   UpdateLRUReplacementState(set,way);


   // Used to update our BIP replacement state.
   UpdateBIPReplacementState(set,way);


   //We will update our winner cache with hits from both LRU and BIP
   for (uint32_t i = 0; i < LLC_WAYS; i++)
   {
   	// If there's a hit in either LRU or BIP, promote inn the Winner cache
   	if (lru[set][i] == 0 || bip[set][i] == 0)
   	{
       	winner[set][i] = 0; // Promote to MRU position
   	}
   	else
   	{
       	// Increment the position if not promoted.
       	if (winner[set][i] < LLC_WAYS - 1)
       	{
           	winner[set][i]++;
       	}
   	}
   }
}


// Use this function to print out your own stats on every heartbeat
void PrintStats_Heartbeat()
{
   // Print heartbeat stats if needed
}


// Use this function to print out your own stats at the end of simulation
void PrintStats()
{
   // Print final stats if needed
}

Current Results:
Team(Hits)		NORMAL(Hits)
84648		84776
47383		47674
345624		344786
371588		367736

Miss Rates    Final Average
.63           .507
.55
.66
.19