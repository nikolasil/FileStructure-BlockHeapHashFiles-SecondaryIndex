#ifndef UTIL_H
#define UTIL_H
#define RECORD_SIZE 96           // the size of each record in bytes
#define SECONDARY_RECORD_SIZE 32 // the size of each secondary record in bytes
#define NB_NUMBER_POS 504        // the position of the block number to the next block
#define NUM_ENTRIES_POS 508      // the position of the integer that holds the number of records in the block

typedef struct
{
  int id;
  char name[15];
  char surname[25];
  char address[50];
} Record;

typedef struct
{
  char surname[25];
  int blockId;
} SecondaryRecord;

Record *createRecord(int id, char name[15], char surname[25], char address[50]);
SecondaryRecord *createSecondaryRecord(char surname[25], int blockId);
int getNumEntries(void *block);                // returns the number of entries of a block
int getNextBlockNumber(void *block);           // returns the pointer of the next block
void setNextBlockNumber(void *block, int ptr); // sets the pointer of the next block to ptr
void *jumpToNextEntry(void *entry);            // returns the next entry
void *jumpToNextEntrySec(void *entry);
int increaseNumEntries(void *block); // increases the number of entries at the end of the block by 1
int deacreaseNumEntries(void *block);
void printEntry(Record *entry); // prints the entry given
void initBlock(void *block);    // initiallize the block

#endif