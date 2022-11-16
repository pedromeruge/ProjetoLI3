#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "files.h"

FILE ** open_cmdfiles (char ** argv) {

    char * dataFolderName = argv[1];
	char * queryFileName = argv[2];

    char fileName[FILEPATH_SIZE]; // buffer para o nome de cada ficheiro de input a abrir
	strcpy(fileName,dataFolderName);
	strcat (fileName,"/users.csv");
	FILE *ptr1 = fopen(fileName, "r");
	if (!ptr1) {
		perror("File opening error");
		exit(1);
	}
	
	strcpy(fileName,dataFolderName);
	strcat (fileName,"/drivers.csv");
	FILE *ptr2 = fopen(fileName, "r");
	if (!ptr2) {
		perror("File opening error");
		exit(1);
	}

	strcpy(fileName,dataFolderName);
	strcat (fileName,"/rides.csv");
	FILE *ptr3 = fopen(fileName, "r");
	if (!ptr3) {
		perror("File opening error");
		exit(1);
	}

	FILE *teste = fopen(queryFileName,"r");
	if (!teste) {
		perror("File opening error");
		exit(1);
	}

    FILE ** retArray = malloc(sizeof(FILE *)*4);
	retArray[0]= ptr1;
	retArray[1]= ptr2;
	retArray[2]= ptr3;
	retArray[3]= teste;
    return retArray;
}
