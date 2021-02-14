#include "heap.h"
#include "../BF/BF.h"
#include "../util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int HP_CreateFile(char *fileName, char attrType, char *attrName, int attrLength) {
  if (access(fileName, F_OK) != 0) {
    int fileDesc;
    void *firstBlock;
    if (BF_CreateFile(fileName) < 0) {
      printf("In heap file named %s\n", fileName);
      BF_PrintError("[HP_CreateFile]Error Creating heap file");
      return -1;
    }
    if ((fileDesc = BF_OpenFile(fileName)) < 0) {
      printf("In heap file named %s\n", fileName);
      BF_PrintError("[HP_CreateFile]Error opening heap file");
      return -1;
    }
    if (BF_AllocateBlock(fileDesc) < 0) {
      printf("In heap file named %s\n", fileName);
      BF_PrintError("[HP_CreateFile]Error Allocating First Block");
      return -1;
    }
    if (BF_ReadBlock(fileDesc, 0, &firstBlock) < 0) {
      printf("In heap file named %s\n", fileName);
      BF_PrintError("[HP_CreateFile]Error getting First Block");
      return -1;
    }

    firstBlockInfo *firstBlockContent = malloc(sizeof(firstBlockInfo));
    strcpy(firstBlockContent->typeOfFile, "HP");
    strcpy(firstBlockContent->filename, fileName);
    strcpy(firstBlockContent->attrName, attrName);
    firstBlockContent->attrType = attrType;
    firstBlockContent->attrLength = attrLength;
    memcpy(firstBlock, firstBlockContent, sizeof(*firstBlockContent));
    if (BF_WriteBlock(fileDesc, 0) < 0) {
      printf("In heap file named %s\n", fileName);
      BF_PrintError("[HP_CreateFile]Error writing to first block");
      return -1;
    }
    free(firstBlockContent);
    if (BF_CloseFile(fileDesc) < 0) {
      printf("In heap file named %s\n", fileName);
      BF_PrintError("[HP_CreateFile]Error closing heap file");
      return -1;
    }
    return 0;
  } else {
    printf("[HP_CreateFile]File with name %s already exists\n", fileName);
    return -1;
  }
  return 0;
}

HP_info *HP_OpenFile(char *fileName) {
  int fileDesc;
  firstBlockInfo *firstBlock;
  if ((fileDesc = BF_OpenFile(fileName)) < 0) {
    printf("In heap file named %s\n", fileName);
    BF_PrintError("[HP_OpenFile]Error opening heap file");
    return NULL;
  }
  void *block;
  if (BF_ReadBlock(fileDesc, 0, &block) < 0) {
    printf("In heap file named %s\n", fileName);
    BF_PrintError("[HP_OpenFile]Error opening heap file");
    return NULL;
  }

  firstBlock = block;
  HP_info *hpinfo = malloc(sizeof(HP_info));
  hpinfo->fileDesc = fileDesc;
  hpinfo->attrType = firstBlock->attrType;
  hpinfo->attrLength = firstBlock->attrLength;
  strcpy(hpinfo->attrName, firstBlock->attrName);
  strcpy(hpinfo->fileName, firstBlock->filename);
  if (BF_WriteBlock(fileDesc, 0) < 0) {
    printf("In heap file named %s\n", fileName);
    BF_PrintError("[HP_OpenFile]Error writing to first block");
    return NULL;
  }

  return hpinfo;
}

int HP_CloseFile(HP_info *header_info) {
  if (BF_CloseFile(header_info->fileDesc) < 0) {
    printf("In heap file named %s\n", header_info->fileName);
    BF_PrintError("[HP_CloseFile]Error closing heap file");
    return -1;
  }
  free(header_info);
  return 0;
}

int HP_InsertEntry(HP_info header_info, Record record) {
  int fileDesc = header_info.fileDesc;
  int numBlocks = BF_GetBlockCounter(fileDesc);

  if (numBlocks > 1) {
    int nextBlockNumber = 1;
    void *currentBlock;
    while (nextBlockNumber != -1) {
      if (BF_ReadBlock(fileDesc, nextBlockNumber, &currentBlock) < 0) {
        printf("In heap file named %s\n", header_info.fileName);
        BF_PrintError("[HP_InsertEntry]Error reading heap block");
        return -1;
      }

      int numEntries = getNumEntries(currentBlock);
      Record *currentEntry = (Record *)currentBlock;
      for (int i = 0; i < numEntries; i++) {
        char result1[10];
        char result2[10];
        sprintf(result1, "%d", currentEntry->id);
        sprintf(result2, "%d", record.id);
        if (strcmp(result1, result2) == 0) {
          printf("[HP_InsertEntry]Id = %d already exists\n", record.id);
          return -1;
        }
        currentEntry = jumpToNextEntry(currentEntry);
      }
      if (BF_WriteBlock(fileDesc, nextBlockNumber) < 0) {
        printf("In heap file named %s\n", header_info.fileName);
        BF_PrintError("[HP_InsertEntry]Error writing heap block");
        return -1;
      }

      nextBlockNumber = getNextBlockNumber(currentBlock);
    }
  }

  if (numBlocks == 1) {
    void *newblock;
    if (BF_AllocateBlock(fileDesc) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error allocating block");
      return -1;
    }
    if (BF_ReadBlock(fileDesc, 1, &newblock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error reading heap block");
      return -1;
    }

    initBlock(newblock);
    *((Record *)newblock) = record;
    increaseNumEntries(newblock);
    printf("Record %d inserted \n", record.id);
    if (BF_WriteBlock(fileDesc, 1) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }

    return 1;
  }

  void *lastblock;
  if (BF_ReadBlock(fileDesc, numBlocks - 1, &lastblock) < 0) {
    printf("In heap file named %s\n", header_info.fileName);
    BF_PrintError("[HP_InsertEntry]Error reading heap block");
    return -1;
  }

  int numEntries = getNumEntries(lastblock);

  if (numEntries < BLOCK_SIZE / RECORD_SIZE) {
    void *newRecordP = lastblock;
    int i;
    for (i = 0; i < numEntries; i++) {
      newRecordP = jumpToNextEntry(newRecordP);
    }
    *((Record *)newRecordP) = record;
    increaseNumEntries(lastblock);
    if (BF_WriteBlock(fileDesc, numBlocks - 1) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }
    printf("Record %d inserted \n", record.id);
    return numBlocks - 1;
  } else {
    void *prevblock;
    void *newblock;
    if (BF_WriteBlock(fileDesc, numBlocks - 1) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }

    if (BF_AllocateBlock(fileDesc) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error allocating block");
      return -1;
    }
    if (BF_ReadBlock(fileDesc, numBlocks, &newblock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error reading heap block 3");
      return -1;
    }

    initBlock(newblock);
    *((Record *)newblock) = record;

    increaseNumEntries(newblock);
    if (BF_WriteBlock(fileDesc, numBlocks) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }

    if (BF_ReadBlock(fileDesc, numBlocks - 1, &prevblock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error reading heap block");
      return -1;
    }

    setNextBlockNumber(prevblock, numBlocks);
    if (BF_WriteBlock(fileDesc, numBlocks - 1) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }
    printf("Record %d inserted \n", record.id);
    return numBlocks;
  }
}

int HP_InsertEntrySec(HP_info header_info, SecondaryRecord record) {
  int fileDesc = header_info.fileDesc;
  int numBlocks = BF_GetBlockCounter(fileDesc);

  if (numBlocks == 1) { //First Block
    void *newblock;
    if (BF_AllocateBlock(fileDesc) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error allocating block");
      return -1;
    }
    if (BF_ReadBlock(fileDesc, 1, &newblock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error reading heap block");
      return -1;
    }

    initBlock(newblock);
    *((SecondaryRecord *)newblock) = record;
    increaseNumEntries(newblock);
    printf("Record %s inserted \n", record.surname);

    if (BF_WriteBlock(fileDesc, 1) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }

    return 1;
  }
  void *lastblock;
  if (BF_ReadBlock(fileDesc, numBlocks - 1, &lastblock) < 0) {
    printf("In heap file named %s\n", header_info.fileName);
    BF_PrintError("[HP_InsertEntry]Error reading heap block");
    return -1;
  }

  int numEntries = getNumEntries(lastblock);

  if (numEntries < BLOCK_SIZE / SECONDARY_RECORD_SIZE - 1) {
    void *newRecordP = lastblock;
    for (int i = 0; i < numEntries; i++) {
      newRecordP = jumpToNextEntrySec(newRecordP);
    }
    *((SecondaryRecord *)newRecordP) = record;
    increaseNumEntries(lastblock);
    if (BF_WriteBlock(fileDesc, numBlocks - 1) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }
    printf("Record %s inserted \n", record.surname);
    return numBlocks - 1;
  } else {
    void *prevblock;
    void *newblock;
    if (BF_WriteBlock(fileDesc, numBlocks - 1) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }

    if (BF_AllocateBlock(fileDesc) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error allocating block");
      return -1;
    }
    if (BF_ReadBlock(fileDesc, numBlocks, &newblock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error reading heap block 3");
      return -1;
    }

    initBlock(newblock);
    *((SecondaryRecord *)newblock) = record;

    increaseNumEntries(newblock);
    if (BF_WriteBlock(fileDesc, numBlocks) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }

    if (BF_ReadBlock(fileDesc, numBlocks - 1, &prevblock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error reading heap block");
      return -1;
    }

    setNextBlockNumber(prevblock, numBlocks);
    if (BF_WriteBlock(fileDesc, numBlocks - 1) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_InsertEntry]Error writing to block");
      return -1;
    }
    printf("Record %s inserted \n", record.surname);
    return numBlocks;
  }
}

int HP_DeleteEntry(HP_info header_info, void *value) {
  int fileDesc = header_info.fileDesc;
  int nextBlockNumber = 1;
  void *currentBlock;
  while (nextBlockNumber != -1) {
    if (BF_ReadBlock(fileDesc, nextBlockNumber, &currentBlock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_DeleteEntry]Error reading heap block");
      return -1;
    }

    Record *currentEntry = (Record *)currentBlock;
    for (int i = 0; i < BLOCK_SIZE / RECORD_SIZE; i++) {
      char result[10];
      sprintf(result, "%d", currentEntry->id);
      if (strcmp(result, value) == 0) {
        printf("Record with id %d deleted\n", currentEntry->id);
        memset(currentEntry, 0, RECORD_SIZE);
        deacreaseNumEntries(currentBlock);
        if (BF_WriteBlock(fileDesc, nextBlockNumber) < 0) {
          printf("In heap file named %s\n", header_info.fileName);
          BF_PrintError("[HP_DeleteEntry]Error writing heap block");
          return -1;
        }

        return 0;
      }
      currentEntry = jumpToNextEntry(currentEntry);
    }
    if (BF_WriteBlock(fileDesc, nextBlockNumber) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_DeleteEntry]Error writing heap block");
      return -1;
    }

    nextBlockNumber = getNextBlockNumber(currentBlock);
  }
  printf("Record with id=%d could not be deleted cause it's not in the heap named %s\n", atoi(value), header_info.fileName);
  return -1;
}
int HP_GetAllEntriesSurname(HP_info header_info, char *surname) {
  void *currentBlock;
  int fileDesc = header_info.fileDesc;
  int numBlocks = BF_GetBlockCounter(fileDesc);
  int nextBlockNumber = 1;

  while (nextBlockNumber != -1) {
    if (BF_ReadBlock(fileDesc, nextBlockNumber, &currentBlock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_GetAllEntries]Error reading heap block");
      return -1;
    }

    Record *currentEntry = (Record *)currentBlock;
    for (int i = 0; i < BLOCK_SIZE / RECORD_SIZE; i++) {
      if (!strcmp(currentEntry->surname, surname)) {
        printEntry(currentEntry);
      }
      currentEntry = jumpToNextEntry(currentEntry);
    }
    if (BF_WriteBlock(fileDesc, nextBlockNumber) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_GetAllEntries]Error writing heap block");
      return -1;
    }
    nextBlockNumber = getNextBlockNumber(currentBlock);
  }
  return numBlocks - 1;
}
int HP_GetAllEntries(HP_info header_info, void *value) {
  void *currentBlock;
  int fileDesc = header_info.fileDesc;
  int numBlocks = BF_GetBlockCounter(fileDesc);
  int nextBlockNumber = 1;
  int flag = 0;

  while (nextBlockNumber != -1) {
    if (BF_ReadBlock(fileDesc, nextBlockNumber, &currentBlock) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_GetAllEntries]Error reading heap block");
      return -1;
    }

    Record *currentEntry = (Record *)currentBlock;
    for (int i = 0; i < BLOCK_SIZE / RECORD_SIZE; i++) {
      char result[10];
      sprintf(result, "%d", currentEntry->id);
      if (strcmp(result, value) == 0) {
        printEntry(currentEntry);
        if (BF_WriteBlock(fileDesc, nextBlockNumber) < 0) {
          printf("In heap file named %s\n", header_info.fileName);
          BF_PrintError("[HP_GetAllEntries]Error writing heap block");
          return -1;
        }

        return nextBlockNumber - 1;
      } else if (strcmp(value, "NULL") == 0) {
        flag = 1;
        printEntry(currentEntry);
      }
      currentEntry = jumpToNextEntry(currentEntry);
    }
    if (BF_WriteBlock(fileDesc, nextBlockNumber) < 0) {
      printf("In heap file named %s\n", header_info.fileName);
      BF_PrintError("[HP_GetAllEntries]Error writing heap block");
      return -1;
    }

    nextBlockNumber = getNextBlockNumber(currentBlock);
  }
  if (!flag) {
    printf("Record deleted or doesn't exist\n");
    return -1;
  }
  return numBlocks - 1;
}

int HP_print_Min_Av_Max_Records(HP_info *header_info, int isSecondary) {
  void *currentBlock;
  int fileDesc = header_info->fileDesc;
  int numBlocks = BF_GetBlockCounter(fileDesc);
  int nextBlockNumber = 1;
  int minRecords = 9999999;
  int maxRecords = 0;
  int sumRecords = 0;
  int recordsInBlock;
  while (nextBlockNumber != -1) {
    if (BF_ReadBlock(fileDesc, nextBlockNumber, &currentBlock) < 0) {
      printf("In heap file named %s\n", header_info->fileName);
      BF_PrintError("[HP_print_Min_Av_Max_Records]Error reading heap block");
      return -1;
    }
    int numEntries = getNumEntries(currentBlock);
    Record *currentEntry = (Record *)currentBlock;
    recordsInBlock = 0;
    for (int i = 0; i < numEntries; i++) {
      sumRecords++;
      recordsInBlock++;
      if (isSecondary) {
        currentEntry = jumpToNextEntrySec(currentEntry);
      } else {
        currentEntry = jumpToNextEntry(currentEntry);
      }
    }
    if (recordsInBlock < minRecords) {
      minRecords = recordsInBlock;
    }
    if (recordsInBlock > maxRecords) {
      maxRecords = recordsInBlock;
    }
    if (BF_WriteBlock(fileDesc, nextBlockNumber) < 0) {
      printf("In heap file named %s\n", header_info->fileName);
      BF_PrintError("[HP_print_Min_Av_Max_Records]Error writing heap block");
      return -1;
    }
    nextBlockNumber = getNextBlockNumber(currentBlock);
  }
  printf("Minimum Number of Records : %d \n", minRecords);
  printf("Maximum Number of Records : %d \n", maxRecords);
  printf("Average Number of Records : %d \n", sumRecords / (numBlocks - 1));
  return 0;
}

int HP_get_Overflow_Blocks(HP_info *header_info, int bucketNumber) {
  int fileDesc = header_info->fileDesc;
  int numBlocks = BF_GetBlockCounter(fileDesc);
  if (numBlocks - 2 <= 2) {
    printf("\nBucket %d has no overflow blocks.\n", bucketNumber);
    return 0;
  }
  printf("\nBucket %d has %d overflow blocks.\n", bucketNumber, numBlocks - 2);
  return 1;
}
