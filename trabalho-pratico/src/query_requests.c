#include <stdlib.h>
#include <stdio.h>
#include "query_requests.h"
#include "query_func.h"
#include <string.h>
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

#define LINE_SIZE 128
#define TEST_PATH "Resultados/command%d_output.txt"
int writeResults (int commandN, char * strResult) {
    int ret1 = EOF;
    char resultPath[64];
	snprintf(resultPath, 64, TEST_PATH, commandN);

    FILE *fpout = fopen(resultPath,"w");
    if (!fpout) {
		perror("Unable to open/create output file");
        return 1;
    }
    if (strResult) ret1 = fputs(strResult, fpout); // para nao dar erro se a string for vazia 
    if (ret1 == EOF) { // caso ficheiro não abra
        return 2;
    }
    fclose(fpout);
    return 0;
}

char * NOP(char *city, char *trash1, char *trash2, UserData *userData, DriverStruct *driverData[], RidesData *ridesData) {
	return NULL;
}

int queryRequests (FILE * fp, UserData *userData, DriverStruct *driverData[], RidesData *ridesData) {
	query_func * queryList[9] = {query_1, query_2, NOP, query_4, NOP, NOP, NOP, NOP, NOP};
    char * strBuffer = malloc(sizeof(char)*LINE_SIZE); // buffer de cada linha lida
    char * querryResult = NULL; // pointer para a string resultante de cada querry
    char * tempsegstr[4]; // array para atribuir o segmento correto do input
    char * strHolder, *temp;
    ssize_t read; size_t len = LINE_SIZE; // para o getline
    int i,j, commandN = 1, writeRet;
    
    // lê linhas individualmente até chegar ao fim do ficheiro
	for (i=0; (read = getline(&strBuffer, &len, fp) != -1); i++, commandN++) {

        strBuffer[strcspn(strBuffer, "\n")] = 0; // para remover o newline

        // mais rápido assim ou fazer só sscanf com vários ifs para cada query??
		temp = strBuffer;
        for (j = 0; j < 4 && (strHolder = strsep(&strBuffer," ")); j++) { // j<4 por segurança
            tempsegstr[j] = strHolder;
        }
		strBuffer = temp;

        querryResult = queryList[(*tempsegstr[0]) - 49] (tempsegstr[1],tempsegstr[2],tempsegstr[3],userData,driverData,ridesData); // -48 para dar o numero correto, -1 para a query 1 dar no lugar 0
		writeRet = writeResults(commandN, querryResult);
        if (writeRet) {
            fprintf(stderr, "error writing output file %d\n", commandN);
            return 1;
        }
		free (querryResult); // free do buffer de output
		len = LINE_SIZE; // após um getline, len é alterado para o tamanho da linha; tem de ser reset, a próxima linha pode ter len maior

    }
    free (strBuffer); // free do buffer de input
    return 0;
}
