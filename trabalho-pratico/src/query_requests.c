#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "query_requests.h"
#include "files.h"
#define MAX_QUERY_INPUTS 3
#define LINE_SIZE 128

char *NOP(char * inputStr[], UserData *userData, DriverData *DriverData, RidesData *ridesData) {
    return NULL;
}

// É da responsabilidade da caller function dar free da string
char * queryAssign(char * queryInput, UserData * userData, DriverData * driverData, RidesData * ridesData, int commandN) { // remover commandN, no futuro, só é usado para o debug
    int i,j;
    query_func *queryList[9] = {query_1, query_2, query_3, query_4, query_5, query_6, query_7, NOP, query_9};
    char *strHolder, *tempsegstr[MAX_QUERY_INPUTS+1]; // array com segmentos de input para uma query (atualizado em cada linha)
    //char *  *temp;
    //temp = queryInput;
    queryInput[strcspn(queryInput, "\n")] = 0; // para remover eventual newline na string de input
    
    tempsegstr[0] = NULL;
    tempsegstr[1] = NULL;
    tempsegstr[2] = NULL;
    tempsegstr[3] = NULL;
    
    for (i = 0; i <= MAX_QUERY_INPUTS && (strHolder = strsep(&queryInput, " ")); i++) {
        tempsegstr[i] = strHolder;
    }
    //strBuffer = temp;

    //### print de debug para os input de uma query
    printf("command (%d), query |%d| input segments:",commandN,(*tempsegstr[0]) - 49 + 1);
    for (j = 1;j <= MAX_QUERY_INPUTS && tempsegstr[j]; j++)
        printf(" <%.16s>",tempsegstr[j]);
    putchar('\n');
    fflush(stdout);
    //### 

    return (queryList[(*tempsegstr[0]) - 49](tempsegstr+1, userData, driverData, ridesData)); // -48 para dar o numero correto, -1 para a query 1 dar no lugar 0
}

//modo interativo de correr queries
// TODO: Error check de inputs para queries no modo interativo
int terminalRequests(UserData *userData, DriverData *driverData, RidesData *ridesData) {
    char *strBuffer = malloc(sizeof(char) * LINE_SIZE), // buffer de cada linha lida
         *queryResult = NULL; // pointer para a string resultante de cada querry
     int commandN = 1; // só para debug, pode-se remover depois

    fprintf(stdout,"\nModo interativo!\nInput para uma querry:\n");

    while(fgets(strBuffer,LINE_SIZE,stdin)) {

        queryResult = queryAssign(strBuffer,userData,driverData,ridesData,commandN);

        if (queryResult == NULL) 
            fprintf(stdout,"\nA query não devolveu nenhum resultado :(\n\n");
        else 
            fprintf(stdout,"\nResultado da query:\n\n%s\n", queryResult);
        fprintf(stdout,"Input para uma querry:\n");

        commandN ++; // só para debug, pode-se remover depois
        free(queryResult);
    }
    fprintf(stdout, "A sair do modo interativo...\n");
    return 0; // pode ser usado para comandos de erro no futuro talvez?
}

//modo batch de correr queries
int fileRequests(FILE *fp, UserData *userData, DriverData *driverData, RidesData *ridesData) {
    char *strBuffer = malloc(sizeof(char) * LINE_SIZE), // buffer de cada linha lida
         *queryResult = NULL;                          // pointer para a string resultante de cada querry
    ssize_t read;
    size_t len = LINE_SIZE; // para o getline
    int i, commandN = 1, writeRet;

    // lê linhas individualmente até chegar ao fim do ficheiro
    for (i = 0; (read = getline(&strBuffer, &len, fp) != -1); i++, commandN++) {

        printf("\x1b[1F\x1b[1F\x1b[2K ");

        queryResult = queryAssign(strBuffer,userData, driverData, ridesData,commandN);
        writeRet = writeResults(commandN, queryResult);
        if (writeRet) { // returns positivos indicam erros na função
            fprintf(stderr, "error writing output file %d\n", commandN);
            return 1;
        }

        free(queryResult); // free da string resultante da query
    }
    free(strBuffer); // free do buffer de input
    return 0;
}



