#include "sht.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int SHT_CreateSecondaryIndex(char *sfileName, char *attrName, int attrLength, int buckets, char *fileName) {
  if (access(sfileName, F_OK)) {
    int fileDesc;
    void *firstBlock;
    if (BF_CreateFile(sfileName) < 0) {
      printf("In hash file named %s\n", sfileName);
      BF_PrintError("[HT_CreateIndex]Error Creating hash file");
      return -1;
    }
    if ((fileDesc = BF_OpenFile(sfileName)) < 0) {
      printf("In hash file named %s\n", sfileName);
      BF_PrintError("[HT_CreateIndex]Error opening hash file");
      return -1;
    }
    if (BF_AllocateBlock(fileDesc) < 0) {
      printf("In hash file named %s\n", sfileName);
      BF_PrintError("[HT_CreateIndex]Error Allocating First Block");
      return -1;
    }
    if (BF_ReadBlock(fileDesc, 0, &firstBlock) < 0) {
      printf("In hash file named %s\n", sfileName);
      BF_PrintError("[HT_CreateIndex]Error getting First Block");
      return -1;
    }
    firstBlockInfoSHT *firstBlockContent = malloc(sizeof(firstBlockInfoSHT));
    strcpy(firstBlockContent->typeOfFile, "HT");
    strcpy(firstBlockContent->sFileName, sfileName);
    strcpy(firstBlockContent->attrName, attrName);
    strcpy(firstBlockContent->fileName, fileName);
    firstBlockContent->attrLength = attrLength;
    firstBlockContent->buckets = buckets;
    memcpy(firstBlock, firstBlockContent, sizeof(*firstBlockContent));
    if (BF_WriteBlock(fileDesc, 0) < 0) {
      printf("In hash file named %s\n", sfileName);
      BF_PrintError("[HT_CreateIndex]Error writing to first block");
      return -1;
    }
    free(firstBlockContent);
    int i;
    for (i = 1; i < HASH_TABLE_SIZE; i++) {
      void *init;
      if (BF_AllocateBlock(fileDesc) < 0) {
        printf("In hash file named %s\n", sfileName);
        BF_PrintError("[HT_CreateIndex]Error Allocating First Block");
        return -1;
      }
      if (BF_ReadBlock(fileDesc, i, &init) < 0) {
        printf("In hash file named %s\n", sfileName);
        BF_PrintError("[HT_CreateIndex]Error getting First Block");
        return -1;
      }
      hashTableEntry *currentBlock = (hashTableEntry *)init;
      currentBlock->heapExists = 0;
      char result[7];
      char *buffer = malloc(strlen("Secondary Bucket") + 5);
      sprintf(result, "%d", i);
      strcpy(buffer, "Secondary Bucket ");
      strcat(buffer, result);
      strcpy(currentBlock->heapFileName, buffer);
      free(buffer);
      if (BF_WriteBlock(fileDesc, i) < 0) {
        printf("In hash file named %s\n", sfileName);
        BF_PrintError("[HT_CreateIndex]Error writing to first block");
        return -1;
      }
    }
    if (BF_CloseFile(fileDesc) < 0) {
      printf("In hash file named %s\n", sfileName);
      BF_PrintError("[HT_CreateIndex]Error closing hash file");
      return -1;
    }
    return 0;
  } else {
    printf("[SHT_CreateIndex]File with name %s already exists\n", sfileName);
    return -1;
  }
}

SHT_info *SHT_OpenSecondaryIndex(char *sfileName) {
  int fileDesc;
  if ((fileDesc = BF_OpenFile(sfileName)) < 0) {
    printf("In hash file named %s\n", sfileName);
    BF_PrintError("[HT_OpenIndex]Error opening hash file");
    return NULL;
  }
  void *init;
  if (BF_ReadBlock(fileDesc, 0, &init) < 0) {
    printf("In hash file named %s\n", sfileName);
    BF_PrintError("[HT_OpenIndex]Error opening hash file");
    return NULL;
  }
  firstBlockInfoSHT *firstBlock = (firstBlockInfoSHT *)init;
  SHT_info *htinfo = malloc(sizeof(SHT_info));
  htinfo->fileDesc = fileDesc;
  htinfo->attrLength = firstBlock->attrLength;
  htinfo->numBuckets = firstBlock->buckets;
  strcpy(htinfo->fileName, firstBlock->fileName);
  strcpy(htinfo->attrName, firstBlock->attrName);
  if (BF_WriteBlock(fileDesc, 0) < 0) {
    printf("In hash file named %s\n", sfileName);
    BF_PrintError("[HT_OpenIndex]Error writing to hash file");
    return NULL;
  }
  return htinfo;
};

int SHT_CloseSecondaryIndex(SHT_info *header_info) {
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

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record) {
  int fileDesc = header_info.fileDesc;
  char *hash = malloc(SHA_DIGEST_LENGTH * sizeof(char));
  generateHashString(record.surname, hash);
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
  int blockInsertedIndex;

  if (!heapExists) {
    HP_CreateFile(block->heapFileName, 's', header_info.attrName, header_info.attrLength);
    HP_info *hpinfo = HP_OpenFile(block->heapFileName);
    block->heapHeaderInfo = hpinfo;
    block->heapExists = 1;
    if (BF_WriteBlock(fileDesc, hashTableIndex) < 0) {
      printf("In hash file named %s\n", header_info.fileName);
      BF_PrintError("[HT_InsertEntry]Error writing to hash table");
      return -1;
    }
    blockInsertedIndex = HP_InsertEntrySec(*(block->heapHeaderInfo), record);
  } else {
    if (BF_WriteBlock(fileDesc, hashTableIndex) < 0) {
      printf("In hash file named %s\n", header_info.fileName);
      BF_PrintError("[HT_InsertEntry]Error writing to hash table");
      return -1;
    }
    blockInsertedIndex = HP_InsertEntrySec(*(block->heapHeaderInfo), record);
  }

  return blockInsertedIndex;
}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value) {
  int fileDesc = header_info_ht.fileDesc;
  int sFileDesc = header_info_sht.fileDesc;
  int blockCounter = 0;

  if (!strcmp(value, "NULL")) { //get all entries from primary index
    return HT_GetAllEntries(header_info_ht, "NULL");
  }
  char *surname = (char *)value;
  char *hash = malloc(SHA_DIGEST_LENGTH * sizeof(char));
  generateHashString(surname, hash);
  int hashTableIndex = getIndex(hash);
  free(hash);
  void *init;

  if (BF_ReadBlock(sFileDesc, hashTableIndex, &init) < 0) {
    printf("In hash file named %s\n", header_info_sht.fileName);
    BF_PrintError("[HT_InsertEntry]Error reading hash table");
    return -1;
  }

  hashTableEntry *block = (hashTableEntry *)init;
  int heapExists = block->heapExists;

  if (BF_WriteBlock(sFileDesc, hashTableIndex) < 0) {
    printf("In hash file named %s\n", header_info_sht.fileName);
    BF_PrintError("[HT_GetAllEntries]Error opening hash table entry");
    return -1;
  }

  if (heapExists) {
    void *currentBlock;
    int internalFileDesc = block->heapHeaderInfo->fileDesc;
    int nextBlockNumber = 1;
    HP_info *internalInfo;
    int blockId;
    void *bucket;
    printf("\n——> Printing all records with surname: %s \n", surname);
    while (nextBlockNumber != -1) {
      if (BF_ReadBlock(internalFileDesc, nextBlockNumber, &currentBlock) < 0) {
        printf("In heap file named %s\n", header_info_sht.fileName);
        BF_PrintError("[SHT_SecondaryGetAllEntries]Error reading heap block");
        return -1;
      }

      SecondaryRecord *currentEntry = (SecondaryRecord *)currentBlock;

      if (BF_WriteBlock(internalFileDesc, nextBlockNumber) < 0) {
        printf("In heap file named %s\n", header_info_sht.fileName);
        BF_PrintError("[SHT_SecondaryGetAllEntries]Error writing heap block");
        return -1;
      }
      nextBlockNumber = getNextBlockNumber(currentBlock); // jump to next block

      for (int i = 0; i < BLOCK_SIZE / SECONDARY_RECORD_SIZE - 1; i++) {
        if (!strcmp(currentEntry->surname, surname)) {
          //get block id
          blockId = currentEntry->blockId;

          if (BF_ReadBlock(fileDesc, blockId, &bucket) < 0) {
            printf("In heap file named %s\n", header_info_sht.fileName);
            BF_PrintError("[SHT_SecondaryGetAllEntries]Error reading heap block");
            return -1;
          }

          hashTableEntry *currentBucket = (hashTableEntry *)bucket;
          internalInfo = currentBucket->heapHeaderInfo;

          if (BF_WriteBlock(fileDesc, blockId) < 0) {
            printf("In heap file named %s\n", header_info_sht.fileName);
            BF_PrintError("[SHT_SecondaryGetAllEntries]Error writing heap block");
            return -1;
          }
          //search bucket for entries with surname
          blockCounter += HP_GetAllEntriesSurname(*internalInfo, surname);
        }
        currentEntry = jumpToNextEntrySec(currentEntry); // jump to next entry
      }
    }
  } else {
    printf("Error finding hash table block to print entry\n");
    return -1;
  }

  return blockCounter;
}
