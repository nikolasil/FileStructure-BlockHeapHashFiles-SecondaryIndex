#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"

#define FILENAME "file"
#define MAX_FILES 1
#define MAX_BLOCKS 200

int main(int argc, char **argv)
{
	int bfs;
	int i, j;
	char filename[5];
	void *block;
	int blkCnt;

	BF_Init();
	strcpy(filename, FILENAME);
	printf("File %s\n", filename);
	if (BF_CreateFile(filename) < 0)
	{
		BF_PrintError("Error creating file");
		exit(EXIT_FAILURE);
	}
	if ((bfs = BF_OpenFile(filename)) < 0)
	{
		BF_PrintError("Error opening file");
		return 0;
	}
	printf("\nbfs = %d\n", bfs);
	for (j = 0; j < MAX_BLOCKS; j++)
	{
		printf("\nbfs = %d\n", bfs);
		printf("Block %d\n", j);
		if (BF_AllocateBlock(bfs) < 0)
		{
			BF_PrintError("Error allocating block");
			return 0;
		}

		blkCnt = BF_GetBlockCounter(bfs);
		printf("File %d has %d blocks\n", bfs, blkCnt);

		if (BF_ReadBlock(bfs, j, &block) < 0)
		{
			BF_PrintError("Error getting block");
			return 0;
		}
		strncpy(block, (int *)&j, sizeof(int));
		printf("j %d\n", abs(*(int *)&j));
		printf("blo %d\n", abs(*((int *)block)));
		if (BF_WriteBlock(bfs, j) < 0)
		{
			BF_PrintError("Error writing block back");
			return 0;
		}
	}
	for (j = 0; j < MAX_BLOCKS; j++)
	{
		if (BF_ReadBlock(bfs, j, &block) < 0)
		{
			BF_PrintError("Error getting block");
			return 0;
		}
		printf("j %d\n", abs(*(int *)&j));
		printf("blo %d\n", abs(*((int *)block)));
		if (BF_WriteBlock(bfs, j) < 0)
		{
			BF_PrintError("Error writing block back");
			return 0;
		}
	}

	// if (BF_CloseFile(bfs) < 0)
	// {
	//   BF_PrintError("Error closing file");
	//   break;
	// }
	return 0;
}
