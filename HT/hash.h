#ifndef HASH_H
#define HASH_H

#include "../BF/BF.h"
#include "../HP/heap.h"
#include "../Hashing/hashing.h"
#include <openssl/sha.h>
typedef struct
{
  int fileDesc;
  char attrType;
  char attrName[20];
  char fileName[20];
  int attrLength;
  long int numBuckets;
} HT_info;

int HT_CreateIndex(char *fileName, char attrType, char *attrName, int attrLength, int buckets);
HT_info *HT_OpenIndex(char *fileName);
int HT_CloseIndex(HT_info *header_info);
int HT_InsertEntry(HT_info header_info, Record record);
int HT_DeleteEntry(HT_info header_info, void *value);
int HT_GetAllEntries(HT_info header_info, void *value);
int HashStatistics(char *filename);
typedef struct
{
  char typeOfFile[2];
  char filename[20];
  char attrType;
  char attrName[20];
  int attrLength;
  int buckets;
} firstBlockInfoHT;
#endif