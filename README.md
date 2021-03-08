# This project was made during my studies in UOA University and especially for the course Implementation of Database Systems.

---

> There are comments all over my code if there is something that i didn't covered here.

> Κάνουμε BF_init() μόνο μία φορά στην main.

---

## Compile & Run

Includes Makefile which compiles all C files with the make command. The program is executed with the command ./prog [-arg1]

- arg1: Select a record file (1,5,10,15 for records \ * K.txt)

---

## Structure of Heap File Block

- Bytes 0-480: Records
- Bytes 504-508: BlockId of the next Block
- Bytes 508-512: Current number of records in Block

---

## Structure of Δομή Hash File Block

The Hash File is implemented via Block File. Each block represents an entry of the Hash Table.

---

## Structure of Secondary Hash File Block

The Secondary Hash File is implemented via Block File. Each block represents an entry in the Secondary Hash Table.

---

## Main Function (main.c)

- Reads the records from the file selected through the argument (1,5,10,15) and enter the record for each line of the record file first in the primary HASH TABLE with key id and then in SECONDARY HASH TABLE with key surname
- Then we print all the entries that were entered.
- And we print all the statistics for both hash tables.

---

## Utils (util.c)

Contains auxiliary functions for retrieving / initializing / changing / printing information in a record or block.

---

## Hashing (hashing.c)

It contains auxiliary functions to create the hash of id through the hash function ** SHA1 ** as well as to calculate the corresponding index in the Hash Table.

---

## Functions HP (Heap File)

### HP_CreateFile

Create File first checks if the filename given to them through access() exists and if it already exists does not continue to create the HP File. After creating and opening a Block File and allocating the first Block we copy the basic information of the Heap File to a struct structure firstBlockInfo. After copying the struct via memcpy() to the first block we call BF_WriteBlock and close the file with BF_CloseFile.

### HP_OpenFile

OpenFile opens the Block File, copies and returns a struct HP_info with the information of the first block. The Heap File remains open until HP_CloseFile.

### HP_CloseFile

CloseFile closes the heap file and releases the struct header_info.

### HP_InsertEntry

InsertEntry, after checking the uniqueness of the Record id given, distinguishes 3 cases:

- There is no record in the Heap (except the 1st block of information) _: Bind the second block, copy the record with memcpy () and increase the number of records of this block.
- The last block has less than 5 records_: We move the pointer to the last record and copy the record having increased the number of records.
- The last block is full (contains 5 entries) : In a similar way to (1) we commit a new block, we copy the record and **we change the NextNumber of the previous block** to show in the blockId of the new block.

### HP_InsertEntrySεc

HP_InsertEntrySec does what it does and HP_InsertEntry simply inserts a secondary record instead of a record.

### HP_DeleteEntry

Runs the entries of each block until it finds the corresponding record id and replaces the record data with 0.

### HP_GetAllEntries

1. **Case value == NULL**: We print all the records of all the blocks
2. **Case value == id**: We print only the record with the corresponding id (1 if we checked the insert for uniqueness id).

### HP_GetAllEntriesSurname

HP_GetAllEntriesSurname is the same as HP_GetAllEntries just looking for surname instead of id.

---

## Functions HT (Hash Table File)

### HT_CreateIndex

Create Index checks if the filename given to them through access () exists and if it already exists does not proceed to the creation of HT File. After creating and opening a Block File and allocating the first block we copy the basic information of the Hash File to a struct structure firstBlockInfoHT. After copying the struct via memcpy () in the first block we initialize each entry (block) of the hash table with the hashTableEntry structure. Call BF_WriteBlock and close the file with BF_CloseFile.

### HT_OpenIndex

OpenFile opens the Block File, copies and returns a struct HT_info with the information of the first block. The Hash File remains open until HT_CloseFile.

### HT_CloseIndex

CloseFile closes the heap file for each entry (block) of the hash table, closes the same hash file (bf_CloseFile) and releases the struct header_info.

### HT_InsertEntry

InsertEntry uses the functions in hashing.c to create the hash and calculate the hash table index and then one of the following:

1. A bucket already exists (heapExists == 1) so it simply calls HP_InsertEntry to that bucket
2. There is no bucket (heapExists == 0) so it creates a heap file, copies to the specific entry of the hash table information about the heap file and imports the entry via HP_InsertEntry.

### HT_DeleteEntry

DeleteEntry calculates the hashIndex of the given record id, goes to the corresponding entry (block) of the hash table
and **only if there is a bucket** calls HP_DeleteEntry.

### HT_GetAllEntries

1. **Case value == NULL**: For each entry of the block file ** in which there is a bucket ** we print all the entries of all the blocks through HP_GetAllEntries.
2. **Case value == id**: Calculate the hashIndex of the record id, go to the corresponding entry (block) and call HP_InsertEntry for the specific id.

### HashStatistics

We run through the hash table blocks that have a bucket and:

- calculate the total number of blocks with totalNumOfBlocks

- Use HP_print_Min_Av_Max_Records (util.c) to calculate the minimum, average and maximum number of records contained in each bucket of the file.

- Calculate the average number of blocks contained in the hash table buckets (totalNumOfBlocks / buckets).

- We use HP_get_Overflow_Blocks (util.c) to calculate the number of overflow buckets and how many blocks there are in each bucket.

---

## Functions SHT (Secondary Hash Table File)

### SHT_CreateSecondaryIndex

SHT_CreateSecondaryIndex checks if the sfilename given to them via access () exists and if it already exists does not proceed to the creation of SET File. After creating and opening a Block File and allocating the first block we copy the basic information of the Secondary Hash File to a struct structure firstBlockInfoSHT. After copying the struct via memcpy () in the first block we initialize each entry (block) of the hash table with the hashTableEntry structure. Call BF_WriteBlock and close the file with BF_CloseFile.

### SHT_OpenSecondaryIndex

SHT_OpenSecondaryIndex opens the Block File, copies and returns a struct SHT_info with the information of the first block. The Secondary Hash File remains open until SHT_CloseSecondaryIndex.

### SHT_CloseSecondaryIndex

SHT_CloseSecondaryIndex closes the heap file for each entry (block) of the secondary hash table, closes the same secondary hash file (bf_CloseFile) and releases the struct header_info.

### SHT_SecondaryInsertEntry

We do not check here for uniqueness as in the Hash Table because there may be two registrations with the same surname.

SHT_SecondaryInsertEntry uses the functions in hashing.c to create the hash and compute the index of the secondary hash table and then one of the following:

1. A bucket already exists (heapExists == 1) so it simply calls HP_InsertEntrySec to that bucket
2. There is no bucket (heapExists == 0) so it creates a heap file, copies to the specific entry of the hash table information about the heap file and enters the entry via HP_InsertEntrySec.


### SHT_SecondaryGetAllEntries

1. **Case value == NULL**: For each entry of the block file ** in which there is a bucket ** Call HT_GetAllEntries with NULL.
2. **Case value == id**: We calculate the hashIndex of the secondary record id, go to the corresponding entry (block) and run the Bucket looking for the surname we are interested in, retrieve the block id that shows in the primary index and call HP_GetAllEntriesSurname () with arguments to the block file information from the primary index bucket and surname.

### HashStatistics

Depending on the filename we choose the way we control the records (+96 bytes for primary index and +32 bytes for secondary index)

We run through the hash table blocks that have a bucket and:

- calculate the total number of blocks with totalNumOfBlocks
- We use HP_print_Min_Av_Max_Records (util.c) to calculate the minimum, average and maximum number of records contained in each bucket of the file.
- Calculate the average number of blocks contained in the hash table buckets (totalNumOfBlocks / buckets).
- We use HP_get_Overflow_Blocks (util.c) to calculate the number of overflow buckets and how many blocks there are in each bucket.
