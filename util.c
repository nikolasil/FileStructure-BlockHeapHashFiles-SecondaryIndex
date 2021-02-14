#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Record *createRecord(int id, char name[15], char surname[25], char address[50]) {
  Record *record = malloc(sizeof(Record));
  record->id = id;
  strcpy(record->address, address);
  strcpy(record->surname, surname);
  strcpy(record->name, name);
  return record;
}

SecondaryRecord *createSecondaryRecord(char surname[25], int blockId) {
  SecondaryRecord *record = malloc(sizeof(SecondaryRecord));
  record->blockId = blockId;
  strcpy(record->surname, surname);
  return record;
}

int getNumEntries(void *block) {
  void *blockPointer = block;
  int *numEntriesP = blockPointer + NUM_ENTRIES_POS;
  return *numEntriesP;
}

int getNextBlockNumber(void *block) {
  return *((int *)(block + NB_NUMBER_POS));
}

void setNextBlockNumber(void *block, int ptr) {
  void *blockPointer = block;
  *((int *)(blockPointer + NB_NUMBER_POS)) = ptr;
}

void *jumpToNextEntry(void *entry) {
  void *blockPointer = entry;
  void *NextRecordP = blockPointer + RECORD_SIZE;
  return NextRecordP;
}
void *jumpToNextEntrySec(void *entry) {
  void *blockPointer = entry;
  void *NextRecordP = blockPointer + SECONDARY_RECORD_SIZE;
  return NextRecordP;
}
int increaseNumEntries(void *block) {
  void *blockPointer = block;
  int *numEntriesP = blockPointer + NUM_ENTRIES_POS;
  *(numEntriesP) = *(numEntriesP) + 1;
  return *(numEntriesP);
}

int deacreaseNumEntries(void *block) {
  void *blockPointer = block;
  int *numEntriesP = blockPointer + NUM_ENTRIES_POS;
  *(numEntriesP) = *(numEntriesP)-1;
  return *(numEntriesP);
}

void initBlock(void *block) {
  void *blockPointer = block;
  setNextBlockNumber(blockPointer, -1);
  int *numEntriesP = blockPointer + NUM_ENTRIES_POS;
  *numEntriesP = 0;
}

void printEntry(Record *entry) {
  if (strcmp(entry->name, "") == 0 || strcmp(entry->address, "") == 0 || strcmp(entry->surname, "") == 0) {
    return;
  }
  printf("Id: %d \n", entry->id);
  printf("Name: %s \n", entry->name);
  printf("Surname: %s \n", entry->surname);
  printf("Address: %s \n", entry->address);
}