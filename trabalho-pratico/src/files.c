#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "files.h"

FILE **open_cmdfiles(char **argv)
{

	char *dataFolderName = argv[1];
	char *queryFileName = argv[2];
	printf("1º arg:<%s> 2ºarg: <%s>\n", dataFolderName, queryFileName);
	char fileName[FILEPATH_SIZE]; // buffer para o nome de cada ficheiro de input a abrir
	strcpy(fileName, dataFolderName);
	strcat(fileName, "/users.csv");
	printf("filename: <%s>\n", fileName);
	FILE *ptr1 = fopen(fileName, "r");
	if (!ptr1)
	{
		fprintf(stderr, "File opening error on file %s\n", fileName);
		exit(1);
	}

	strcpy(fileName, dataFolderName);
	strcat(fileName, "/drivers.csv");
	printf("filename: <%s>\n", fileName);
	FILE *ptr2 = fopen(fileName, "r");
	if (!ptr2)
	{
		fprintf(stderr, "File opening error on file %s\n", fileName);
		exit(1);
	}

	strcpy(fileName, dataFolderName);
	strcat(fileName, "/rides.csv");
	printf("filename: <%s>\n", fileName);
	FILE *ptr3 = fopen(fileName, "r");
	if (!ptr3)
	{
		fprintf(stderr, "File opening error on file %s\n", fileName);
		exit(1);
	}

	FILE *querries = fopen(queryFileName, "r");
	if (!querries)
	{
		fprintf(stderr, "File opening error on file %s\n", fileName);
		exit(1);
	}

	FILE **retArray = malloc(sizeof(FILE *) * 4);
	retArray[0] = ptr1;
	retArray[1] = ptr2;
	retArray[2] = ptr3;
	retArray[3] = querries;
	return retArray;
}
