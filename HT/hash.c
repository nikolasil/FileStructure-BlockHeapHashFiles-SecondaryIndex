#include "hash.h"
#include "../util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int HT_CreateIndex(char *fileName, char attrType, char *attrName, int attrLength, int buckets) {
  if (access(fileName, F_OK) != 0) {
    int fileDesc;
    void *firstBlock;
    if (BF_CreateFile(fileName) < 0) {
      printf("In hash file named %s\n", fileName);
      BF_PrintError("[HT_CreateIndex]Error Creating hash file");
      return -1;
    }
    if ((fileDesc = BF_OpenFile(fileName)) < 0) {
      printf("In hash file named %s\n", fileName);
      BF_PrintError("[HT_CreateIndex]Error opening hash file");
      return -1;
    }
    if (BF_AllocateBlock(fileDesc) < 0) {
      printf("In hash file named %s\n", fileName);
      BF_PrintError("[HT_CreateIndex]Error Allocating First Block");
      return -1;
    }
    if (BF_ReadBlock(fileDesc, 0, &firstBlock) < 0) {
      printf("In hash file named %s\n", fileName);
      BF_PrintError("[HT_CreateIndex]Error getting First Block");
      return -1;
    }
    firstBlockInfoHT *firstBlockContent = malloc(sizeof(firstBlockInfoHT));
    strcpy(firstBlockContent->typeOfFile, "HT");
    strcpy(firstBlockContent->filename, fileName);
    strcpy(firstBlockContent->attrName, attrName);
    firstBlockContent->attrType = attrType;
    firstBlockContent->attrLength = attrLength;
    firstBlockContent->buckets = buckets;
    memcpy(firstBlock, firstBlockContent, sizeof(*firstBlockContent));
    if (BF_WriteBlock(fileDesc, 0) < 0) {
      printf("In hash file named %s\n", fileName);
      BF_PrintError("[HT_CreateIndex]Error writing to first block");
      return -1;
    }
    free(firstBlockContent);
    int i = 1;
    for (i = 1; i < HASH_TABLE_SIZE; i++) {
      void *init;
      if (BF_AllocateBlock(fileDesc) < 0) {
        printf("In hash file named %s\n", fileName);
        BF_PrintError("[HT_CreateIndex]Error Allocating First Block");
        return -1;
      }
      if (BF_ReadBlock(fileDesc, i, &init) < 0) {
        printf("In hash file named %s\n", fileName);
        BF_PrintError("[HT_CreateIndex]Error getting First Block");
        return -1;
      }
      hashTableEntry *currentBlock = (hashTableEntry *)init;
      currentBlock->heapExists = 0;
      char result[7];
      char *buffer = malloc(strlen("Bucket") + 5);
      sprintf(result, "%d", i);
      strcpy(buffer, "Bucket ");
      strcat(buffer, result);
      strcpy(currentBlock->heapFileName, buffer);
      free(buffer);
      if (BF_WriteBlock(fileDesc, i) < 0) {
        printf("In hash file named %s\n", fileName);
        BF_PrintError("[HT_CreateIndex]Error writing to first block");
        return -1;
      }
    }
    if (BF_CloseFile(fileDesc) < 0) {
      printf("In hash file named %s\n", fileName);
      BF_PrintError("[HT_CreateIndex]Error closing hash file");
      return -1;
    }
    return 0;
  } else {
    printf("[HT_CreateIndex]File with name %s already exists\n", fileName);
    return -1;
  }
  return 0;
}

HT_info *HT_OpenIndex(char *fileName) {

  int fileDesc;
  if ((fileDesc = BF_OpenFile(fileName)) < 0) {
    printf("In hash file named %s\n", fileName);
    BF_PrintError("[HT_OpenIndex]Error opening hash file");
    return NULL;
  }
  void *init;
  if (BF_ReadBlock(fileDesc, 0, &init) < 0) {
    printf("In hash file named %s\n", fileName);
    BF_PrintError("[HT_OpenIndex]Error opening hash file");
    return NULL;
  }
  firstBlockInfoHT *firstBlock = (firstBlockInfoHT *)init;
  HT_info *htinfo = malloc(sizeof(HT_info));
  htinfo->fileDesc = fileDesc;
  htinfo->attrType = firstBlock->attrType;
  htinfo->attrLength = firstBlock->attrLength;
  htinfo->numBuckets = firstBlock->buckets;
  strcpy(htinfo->fileName, firstBlock->filename);
  strcpy(htinfo->attrName, firstBlock->attrName);
  if (BF_WriteBlock(fileDesc, 0) < 0) {
    printf("In hash file named %s\n", fileName);
    BF_PrintError("[HT_OpenIndex]Error writing to hash file");
    return NULL;
  }
  return htinfo;
}

int HT_CloseIndex(HT_info *header_info) {

  void *init;
  int fileDesc = header_info->fileDesc;

  for (int i = 1; i < HASH_TABLE_SIZE; i++) {
    if (BF_ReadBlock(fileDesc, i, &init) < 0) {
      printf("In hash file named %s\n", header_info->fileName);
      BF_PrintError("[HT_CloseIndex]Error reading hash block");
      return -1;
    }

    hashTableEntry *block = (hashTableEntry *)init;
    if (block->heapExists) {
      HP_CloseFile(block->heapHeaderInfo);
    }
    if (BF_WriteBlock(fileDesc, i) < 0) {
      printf("In hash file named %s\n", header_info->fileName);
      BF_PrintError("[HT_CloseIndex]Error writing hash block");
      return -1;
    }
  }
  if (BF_CloseFile(header_info->fileDesc) < 0) {
    printf("In hash file named %s\n", header_info->fileName);
    BF_PrintError("[HT_CloseIndex]Error closing hash file");
    return -1;
  }
  free(header_info);
  return 0;
}

int HT_InsertEntry(HT_info header_info, Record record) {
  int fileDesc = header_info.fileDesc;
  char *hash = malloc(SHA_DIGEST_LENGTH * sizeof(char));
  generateHash(record.id, hash);
  int hashTableIndex = getIndex(hash);
  free(hash);
  void *init;

  if (BF_ReadBlock(fileDesc, hashTableIndex, &init) < 0) {
    printf("In hash file named %s\n", header_info.fileName);
    BF_PrintError("[HT_InsertEntry]Error reading hash table");
    return -1;
  }
  hashTableEntry *block = (hashTableEntry *)init;
  int heapExists = block->heapExists;
  //int blockInsertedIndex;
  if (!heapExists) {
    HP_CreateFile(block->heapFileName, header_info.attrType, header_info.attrName, header_info.attrLength);
    HP_info *hpinfo = HP_OpenFile(block->heapFileName);
    block->heapHeaderInfo = hpinfo;
    block->heapExists = 1;
    if (BF_WriteBlock(fileDesc, hashTableIndex) < 0) {
      printf("In hash file named %s\n", header_info.fileName);
      BF_PrintError("[HT_InsertEntry]Error writing to hash table");
      return -1;
    }
    HP_InsertEntry(*(block->heapHeaderInfo), record);
  } else {
    if (BF_WriteBlock(fileDesc, hashTableIndex) < 0) {
      printf("In hash file named %s\n", header_info.fileName);
      BF_PrintError("[HT_InsertEntry]Error writing to hash table");
      return -1;
    }
    HP_InsertEntry(*(block->heapHeaderInfo), record);
  }

  return hashTableIndex;
}

int HT_DeleteEntry(HT_info header_info, void *value) {
  int fileDesc = header_info.fileDesc;
  char *hash = malloc(SHA_DIGEST_LENGTH);
  generateHash(atoi(value), hash);
  int hashTableIndex = getIndex(hash);
  free(hash);
  void *init;
  if (BF_ReadBlock(fileDesc, hashTableIndex, &init) < 0) {
    printf("In hash file named %s\n", header_info.fileName);
    BF_PrintError("[HT_DeleteEntry]Error reading hash table");
    return -1;
  }
  hashTableEntry *block = (hashTableEntry *)init;
  int heapExists = block->heapExists;
  if (heapExists) {
    if (BF_WriteBlock(fileDesc, hashTableIndex) < 0) {
      printf("In hash file named %s\n", header_info.fileName);
      BF_PrintError("[HT_DeleteEntry]Error writing to hash table");
      return -1;
    }
    HP_DeleteEntry(*(block->heapHeaderInfo), value);
    return 0;
  } else {
    printf("Entry with id=%s does not exist so nothing was deleted\n", (char *)value);
    if (BF_WriteBlock(fileDesc, hashTableIndex) < 0) {
      printf("In hash file named %s\n", header_info.fileName);
      BF_PrintError("[HT_DeleteEntry]Error writing to hash table");
      return -1;
    }
    return -1;
  }
}

int HT_GetAllEntries(HT_info header_info, void *value) {
  int fileDesc = header_info.fileDesc;
  int blockCounter = 0;
  if (strcmp(value, "NULL") != 0) {
    printf("\n——> Printing all Records with id: %s \n", (char *)value);
    char *hash = malloc(SHA_DIGEST_LENGTH);
    generateHash(atoi(value), hash);
    int hashTableIndex = getIndex(hash);
    free(hash);
    void *init;
    if (BF_ReadBlock(fileDesc, hashTableIndex, &init) < 0) {
      printf("In hash file named %s\n", header_info.fileName);
      BF_PrintError("[HT_GetAllEntries]Error reading hash table");
      return -1;
    }

    hashTableEntry *block = (hashTableEntry *)init;
    int heapExists = block->heapExists;
    int idBlockNumber;

    if (BF_WriteBlock(fileDesc, hashTableIndex) < 0) {
      printf("In hash file named %s\n", header_info.fileName);
      BF_PrintError("[HT_GetAllEntries]Error writing hash table");
      return -1;
    }

    if (heapExists) {
      idBlockNumber = HP_GetAllEntries(*(block->heapHeaderInfo), value);

      if (idBlockNumber == -1) {
        return -1;
      }
      return idBlockNumber;
    } else {
      printf("Error finding hash table block to print entry\n");
      return -1;
    }
  } else {
    void *init;
    for (int i = 1; i < HASH_TABLE_SIZE; i++) {
      if (BF_ReadBlock(fileDesc, i, &init) < 0) {
        printf("In hash file named %s\n", header_info.fileName);
        BF_PrintError("[HT_GetAllEntries]Error reading hash block");
        return -1;
      }
      if (BF_WriteBlock(fileDesc, i) < 0) {
        printf("In hash file named %s\n", header_info.fileName);
        BF_PrintError("[HT_GetAllEntries]Error writing hash block");
        return -1;
      }
      hashTableEntry *currentBlock = (hashTableEntry *)init;
      int heapExists = currentBlock->heapExists;
      if (heapExists) {
        printf("\nIn bucket %d:\n\n", i);
        blockCounter += HP_GetAllEntries(*(currentBlock->heapHeaderInfo), "NULL");
      }
    }
    return blockCounter;
  }
}

int HashStatistics(char *filename) {
  if (!strcmp(filename, "file")) {
    printf("\n\n——> Printing Hash Statistics for Primary Index\n\n");
  } else if (!strcmp(filename, "sfile")) {
    printf("\n\n——> Printing Hash Statistics for Secondary Index\n\n");
  }
  int fileDesc;
  if ((fileDesc = BF_OpenFile(filename)) < 0) {
    printf("In hash file named %s\n", filename);
    BF_PrintError("[HashStatistics]Error opening hash file");
    return -1;
  }
  void *init;
  int totalNumOfBlocks = 0;
  int buckets = 0;
  int overflowBuckets = 0;
  for (int i = 1; i < HASH_TABLE_SIZE; i++) {
    if (BF_ReadBlock(fileDesc, i, &init) < 0) {
      printf("In hash file named %s\n", filename);
      BF_PrintError("[HashStatistics]Error reading hash block");
      return -1;
    }
    hashTableEntry *currentBlock = (hashTableEntry *)init;
    int heapExists = currentBlock->heapExists;

    if (heapExists) {
      totalNumOfBlocks += BF_GetBlockCounter(currentBlock->heapHeaderInfo->fileDesc); //question a
      if (BF_WriteBlock(fileDesc, i) < 0) {
        printf("\nIn hash file named %s\n", filename);
        BF_PrintError("[HashStatistics]Error writing hash block");
        return -1;
      }
      printf("\n ——> Statistics for Bucket %d \n", i); //question b
      if (!strcmp(filename, "file")) {
        HP_print_Min_Av_Max_Records(currentBlock->heapHeaderInfo, 0);
      } else if (!strcmp(filename, "sfile")) {
        HP_print_Min_Av_Max_Records(currentBlock->heapHeaderInfo, 1);
      }
      buckets++; //question c
      if (BF_ReadBlock(fileDesc, i, &init) < 0) {
        printf("\nIn hash file named %s\n", filename);
        BF_PrintError("[HashStatistics]Error reading hash block");
        return -1;
      }
      hashTableEntry *currentBlock = (hashTableEntry *)init;
      if (HP_get_Overflow_Blocks(currentBlock->heapHeaderInfo, i)) {
        overflowBuckets++;
      }
    }
    if (BF_WriteBlock(fileDesc, i) < 0) {
      printf("\nIn hash file named %s\n", filename);
      BF_PrintError("[HashStatistics]Error writing hash block");
      return -1;
    }
  }
  printf("\n\n ———> General Hash Table Statistics \n\n");
  printf("\nThere are %d blocks in each bucket in average \n", totalNumOfBlocks / buckets);
  printf("\nThere are %d overflow buckets \n", overflowBuckets);

  if (BF_CloseFile(fileDesc) < 0) {
    printf("In hash file named %s\n", filename);
    BF_PrintError("[HashStatistics]Error closing hash file");
    return -1;
  }
  return 0;
}