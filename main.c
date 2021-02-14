#include "HP/heap.h"
#include "HT/hash.h"
#include "SHT/sht.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_OF_BUCKETS 9

int main(int argc, char *argv[]) {
  BF_Init();
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  Record *record;
  SecondaryRecord *secRecord;
  int blockId;
  char path[30] = "records/records";
  if (argc != 2) {
    printf("Expected argument: 1 for records1K.txt or 5 for records5K.txt or 10 for records10K.txt or 15 for records15K.txt\n");
    return -1;
  }
  if (!(!strcmp(argv[1], "1") || !strcmp(argv[1], "5") || !strcmp(argv[1], "10") || !strcmp(argv[1], "15"))) {
    printf("Expected for argument 1 for records1K.txt or 5 for records5K.txt or 10 for records10K.txt or 15 for records15K.txt\n");
    return -1;
  }

  strcat(path, argv[1]);
  strcat(path, "K.txt");
  fp = fopen(path, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  printf("Creating hash file with name %s\n", "file");
  int hashTableCreated = HT_CreateIndex("file", 'i', "id", 10, NUM_OF_BUCKETS);
  HT_info *htInfo = HT_OpenIndex("file");
  if (htInfo == NULL) {
    printf("Could not open file\n");
    return -1;
  }
  if (hashTableCreated == -1) {
    fprintf(stderr, "Couldn't create Hash Table\n");
    exit(EXIT_FAILURE);
  }

  printf("Creating secondary hash file with name %s \n", "sfile");
  int sHashTableCreated = SHT_CreateSecondaryIndex("sfile", "surname", 25, NUM_OF_BUCKETS, "file");
  SHT_info *sHtInfo = SHT_OpenSecondaryIndex("sfile");
  if (sHtInfo == NULL) {
    printf("Could not open secondary file\n");
    return -1;
  }
  if (sHashTableCreated == -1) {
    fprintf(stderr, "Couldn't create Secondary Hash Table\n");
    exit(EXIT_FAILURE);
  }

  printf("Importing %s in file\n", path);
  while ((read = getline(&line, &len, fp)) != -1) {

    char extract[read];
    sscanf(line, "{%[^}]", extract);
    const char s[2] = ",";
    char *token;

    /* get the first token */
    token = strtok(extract, s);
    int id;
    char name[15], surname[25], address[50];
    id = atoi(token);
    strcpy(name, strtok(NULL, s));
    sscanf(name, "    \"%[^\"]\"  ", name);

    strcpy(surname, strtok(NULL, s));
    sscanf(surname, "    \"%[^\"]\"  ", surname);

    strcpy(address, strtok(NULL, s));
    sscanf(address, "    \"%[^\"]\"  ", address);

    record = createRecord(id, name, surname, address);

    printf("\n——> Inserting record %d to Primary Index \n", record->id);
    blockId = HT_InsertEntry(*htInfo, *record);

    secRecord = createSecondaryRecord(surname, blockId);

    printf("\n——>Inserting record %s to Secondary Index \n", surname);
    SHT_SecondaryInsertEntry(*sHtInfo, *secRecord);

    char idS[10];
    sprintf(idS, "%d", record->id);
    HT_GetAllEntries(*htInfo, idS);
    SHT_SecondaryGetAllEntries(*sHtInfo, *htInfo, surname);
    printf("\n—————————————————————————————————————————————\n");
    free(record);
    free(secRecord);
  }

  printf("——> Records Imported\n");
  //Get all entries from indexes
  //HT_GetAllEntries(*htInfo, "NULL");
  //SHT_SecondaryGetAllEntries(*sHtInfo, *htInfo, "NULL");

  //Get statistics from indexes
  HashStatistics("file");
  HashStatistics("sfile");
  HT_CloseIndex(htInfo);
  SHT_CloseSecondaryIndex(sHtInfo);

  printf("————— File Cleanup ————— \n");

  for (int i = 1; i <= NUM_OF_BUCKETS; i++) {
    char *bucketName = malloc(sizeof(char) * 14);
    char bucketIndex[2];
    sprintf(bucketIndex, "%d", i);
    strcpy(bucketName, "Bucket ");
    strcat(bucketName, bucketIndex);
    strcat(bucketName, "\0");
    printf("Removing: %s \n", bucketName);
    remove(bucketName);
    free(bucketName);
  }
  for (int i = 1; i <= NUM_OF_BUCKETS; i++) {
    char *bucketName = malloc(sizeof(char) * 50);
    char bucketIndex[2];
    sprintf(bucketIndex, "%d", i);
    strcpy(bucketName, "Secondary Bucket ");
    strcat(bucketName, bucketIndex);
    strcat(bucketName, "\0");
    printf("Removing: %s \n", bucketName);
    remove(bucketName);
    free(bucketName);
  }
  printf("Removing Hash File \n");
  remove("file");

  printf("Removing Secondary Hash File \n");
  remove("sfile");

  fclose(fp);
  if (line) {
    free(line);
    exit(EXIT_SUCCESS);
  }
  return 0;
}
