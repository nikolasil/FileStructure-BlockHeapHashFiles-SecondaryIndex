#ifndef SHT_H
#define SHT_H
#include "../HT/hash.h"
#include "../util.h"

typedef struct
{
  int fileDesc;        /* αναγνωριστικός αριθµός ανοίγµατος αρχείου από το επίπεδο block */
  char attrName[20];   /* το όνοµα του πεδίου που είναι κλειδί για το συγκεκριµένο αρχείο */
  int attrLength;      /* το µέγεθος του πεδίου που είναι κλειδί για το συγκεκριµένο αρχείο */
  long int numBuckets; /* το πλήθος των “κάδων” του αρχείου κατακερµατισµού */
  char fileName[20];   /* όνοµα αρχείου µε το πρωτεύον ευρετήριο στο id */
} SHT_info;

typedef struct
{
  char typeOfFile[2];
  char fileName[20];
  char sFileName[20];
  char attrName[20];
  int attrLength;
  int buckets;
} firstBlockInfoSHT;

int SHT_CreateSecondaryIndex(char *sfileName, char *attrName, int attrLength, int buckets, char *fileName);

SHT_info *SHT_OpenSecondaryIndex(char *sfileName /* όνομα αρχείου */);

int SHT_CloseSecondaryIndex(SHT_info *header_info);

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record);

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value);
#endif