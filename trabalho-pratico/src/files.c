#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "files.h"

// abre os ficheiros de input
// file[3] == NULL em interactive mode
FILE **open_cmdfiles(int argc, char **argv) {
	char * dataFolderName, * queryFileName = NULL;
	short int numberOfInputFiles = N_OF_INPUT_FILES;
	//receber o path dos dados de input
	if (argc == 1) {
		printf("Path da pasta dos dados:\n");
		dataFolderName = malloc(sizeof(char) * FILEPATH_SIZE);
		if (dataFolderName == NULL) exit(1);

		if ((fgets(dataFolderName,FILEPATH_SIZE,stdin)) == NULL) exit(2);
		// strcpy(dataFolderName,"Datasets/fase1/data-regular");
		dataFolderName[strcspn(dataFolderName, "\n")] = 0; // remover o '\n' do input
	} else if (argc == 3) {
		dataFolderName = argv[1];
		queryFileName = argv[2];
		numberOfInputFiles ++; // permite correr mais uma vez o loop abaixo, para abrir também o ficheiro de input das queries
	} else {
		fprintf(stderr,"Only accepted 1 or 3 inputs for program\n");
		exit(3);
	}
	//fprintf(stderr,"exec-path: <%s>, 1º arg:<%s> 2ºarg: <%s>\n", argv[0], dataFolderName, queryFileName);

	//abrir os ficheiros para o path de dados recebido
	short int i;
	char fileName[FILEPATH_SIZE], // buffer para o nome de cada ficheiro de input a abrir
	     * array[N_OF_INPUT_FILES+1] = {"/users.csv","/drivers.csv","/rides.csv",queryFileName};
	FILE ** filePtrArray = malloc(sizeof(FILE *) *(N_OF_INPUT_FILES+1));
	filePtrArray[3] = NULL; // se o ficheiro de input nunca for carregado (modo interativo), fica NULL

	for(i = 0; i < numberOfInputFiles; i++) {
		if (i < N_OF_INPUT_FILES) { // só concatena os ficheiros de dados de input; não concantena o ficheiro das queries de input
			strcpy(fileName, dataFolderName);
			strcat(fileName, array[i]);
		}
		else strcpy(fileName, queryFileName);
		
		fprintf(stderr,"filename: <%s>\n", fileName);
		filePtrArray[i] = fopen(fileName, "r");
		if (filePtrArray[i] == NULL) {
			fprintf(stderr, "File opening error on file %s\n", fileName);
			exit(i+5);
		}
	}

	return filePtrArray;
}

int writeResults(int commandN, char *strResult) {
    int ret1 = EOF;
    char resultPath[64];
    snprintf(resultPath, 64, RESULT_PATH, commandN);

    // printf("command (%d), file output NULL?: [%d]\n",commandN,strResult == NULL);

    FILE *fpout = fopen(resultPath, "w");
    if (!fpout)
    {
        fprintf(stderr,"Open/create output file error, path: %s\n", resultPath);
        return 1;
    }
    if (strResult) {
        ret1 = fputs(strResult, fpout); // para nao dar erro se a string for vazia
        if (ret1 == EOF) { // caso ficheiro não abra
            fprintf(stderr, "Write on output file error\n");
            return 2;
        }
    }
    fclose(fpout);
    return 0;
}
