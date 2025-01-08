// my_predictor.h
// This file contains a sample my_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 32 kilobytes available
// for the CBP-2 contest; it is just an example.

/*
Branch Predictor:
Nathan Portillo
Benjamin Cruz

TeamTeam
*/


class my_update : public branch_update {
public:
   unsigned int index; // Holds the index used for prediction.
};


class my_predictor : public branch_predictor {
public:
#define NUMBER_OF_HISTORY_TABLES    4 //Multiple tables to pick best history length for it's purpose.


#define HISTORY_LENGTH      15 //15, smaller history should possibly yield better results for early branches.
#define HISTORY_LENGTH_2    65 // Intermediary lengths
#define HISTORY_LENGTH_3    75 // Intermediary lengths
#define HISTORY_LENGTH_4    85 // Largest length for theorertically best history
#define HISTORY_LENGTH_5    35 //More than 4 tables didn't seem to yield better results
#define PREDICTION_BITS 23 // Number of bits used for indexing the prediction table -- Changed by hand to give optimal average MPKI


   my_update u;          // Hold update info
   branch_info bi;       // Hold branch info


   unsigned int history_tables[NUMBER_OF_HISTORY_TABLES]; //Initialize a history_tables table.
   unsigned int history_lengths[NUMBER_OF_HISTORY_TABLES] = {HISTORY_LENGTH,HISTORY_LENGTH_2,HISTORY_LENGTH_3,HISTORY_LENGTH_4}; //History length values
   unsigned char tab[NUMBER_OF_HISTORY_TABLES][1<<PREDICTION_BITS]; // Prediction tables from the original code based on the new number of history tables
   int best_historylength_flag = 0; //Create a 'flag' to pick the best history length
  
   /*
       A helper function to try and target specific branches in the code. Kinda works
   */
   bool targettedBranch(int indexOfTable) {
   return (indexOfTable % 3 == 0); //Target every branch divisble by 3. A random heuristic that kind of worked at improving MKPI
   }


   // Initializes history and prediction tables.
   my_predictor(void) {
       int i = 0;
       while (i < NUMBER_OF_HISTORY_TABLES) { //For all 4 tables...
           if (targettedBranch(i)) { //If it is a targetted branch(somewhat random heuristic, trial and error
               memset(tab[i], 2, sizeof(tab[i])); //Initialize the confidence of possible path that can be taken or not.
           } else {
               memset(tab[i], 0, sizeof(tab[i])); //Default case innitialization
           }
           history_tables[i] = 0;
           i++;
       }
   }


branch_update *predict(branch_info & b) { //Prediction function
   bi = b;
   int longest_history = 0; //Used to keep track of history


   for (int i = 0; i < NUMBER_OF_HISTORY_TABLES; ++i) { //Loop through the tables.
       unsigned int index = (history_tables[i] << (PREDICTION_BITS - history_lengths[i])) //Hashing Method, XOR seems to work best still.
                           ^ (b.address & ((1<<PREDICTION_BITS)-1));
       unsigned char predict = tab[i][index] >> 1; //Retrieve the prediction
       if (predict != (b.address & 1)) { //If we encounter a misprediction...
           longest_history = i; //Track the table with the longest history before mispredicting.
       }
   }
  
   best_historylength_flag = longest_history;// Update best_historylength_flag with the index of the table with the longest matching history


   // Use best_historylength_flag to access the correct table for prediction
   u.index = (history_tables[best_historylength_flag] << (PREDICTION_BITS - history_lengths[best_historylength_flag])) //Same hashing but with the best history table
             ^ (b.address & ((1<<PREDICTION_BITS)-1));
   u.direction_prediction(tab[best_historylength_flag][u.index] >> 1); //Retrieve if taken or not taken from the best table.
   return &u;
}




   /*
       Update prediction and history table
   */
   void update(branch_update *u, bool taken, unsigned int target) {
       if (bi.br_flags & BR_CONDITIONAL) {
           my_update *ptr = (my_update*)u;
           unsigned int confidence = ptr->index;
           if (taken) { //If a branch is taken
               if (tab[best_historylength_flag][confidence] < 5) tab[best_historylength_flag][confidence]++; //Change value to less than 5, gives best results for confidence range.
           } else {
               if (tab[best_historylength_flag][confidence] > 0) tab[best_historylength_flag][confidence]--; //Confidence value is fine at 0.
           }
           for (int i = 0; i < NUMBER_OF_HISTORY_TABLES; ++i) { //Update the history tables accordingly.
               history_tables[i] <<= 1;
               history_tables[i] |= taken;
               history_tables[i] &= (1<<history_lengths[i])-1;
           }
       }
   }
};









