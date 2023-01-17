#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "query_requests.h"
#include "files.h"
#define MAX_QUERY_INPUTS 3
#define TOTAL_QUERIES_NUMBER 9
#define LINE_SIZE 128

char *NOP(char * inputStr[], UserData *userData, DriverData *DriverData, RidesData *ridesData) {
    return NULL;
}

// É da responsabilidade da caller function dar free da string
char * queryAssign(char * queryInput, UserData * userData, DriverData * driverData, RidesData * ridesData, int commandN) { // remover commandN, no futuro, só é usado para o debug
    int i;
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
    // printf("command (%d), query |%d| input segments:",commandN,(*tempsegstr[0]) - 49 + 1);
	printf("%d:%d ", commandN,(*tempsegstr[0]) - 49 + 1);

	// int j;
    // for (j = 1;j <= MAX_QUERY_INPUTS && tempsegstr[j]; j++)
    //     printf(" <%.16s>",tempsegstr[j]);
    // putchar('\n');
    //### 

    return (queryList[(*tempsegstr[0]) - 49](tempsegstr+1, userData, driverData, ridesData)); // -48 para dar o numero correto, -1 para a query 1 dar no lugar 0
}

//ações para quando o user escreve "help", no modo interativo
void helpCommands (void) {
    int i;
    char * queryFormats[TOTAL_QUERIES_NUMBER] = {"1 <ID>\" ou \"1 <username>", "2 <N>", "3 <N>", "4 <city>", "5 <dateA> <dateB>", "6 <city> <data A> <data B>","7 <N> <city>", "8 <gender> <X>","9 <data A> <data B>"};

    char * queryDescriptions[TOTAL_QUERIES_NUMBER] = {"Listar o resumo de um perfil registado no serviço através do seu identificador (user -> username; condutor -> ID)",
                                                      "Listar os N condutores com maior avaliação média",
                                                      "Listar os N utilizadores com maior distância viajada",
                                                      "Preço médio das viagens (sem considerar gorjetas) numa determinada cidade",
                                                      "Preço médio das viagens (sem considerar gorjetas) num dado intervalo de tempo",
                                                      "Distância média percorrida, numa determinada cidade, num dado intervalo de tempo",
                                                      "Top N condutores numa determinada cidade",
                                                      "Listar todas as viagens nas quais o utilizador e o condutor são do género passado como parâmetro (M ou F) e têm perfis com X ou mais anos",
                                                      "Listar todas as viagens nas quais um passageiro deu gorjeta, no intervalo de tempo"};

    char * queryExamples [TOTAL_QUERIES_NUMBER] = {"1 7141\" ou \"1 SaCruz110", "2 100","3 100","4 Braga","5 17/06/2015 01/05/2016","6 Lisboa 17/08/2018 11/04/2019","7 1032 Faro","8 F 12","9 24/12/2021 25/12/2021" };

    printf("\x1B[1;1H\x1B[2J"); // limpar a tela
    printf("\n<AJUDA>\n\nQueries disponíveis: %d\n\n",TOTAL_QUERIES_NUMBER);
    for (i = 0;i < TOTAL_QUERIES_NUMBER; i++) {
        printf("Q%d| Formato: \"%s\"\n    Descrição: %s\n    Exemplo: \"%s\"\n\n", i+1, queryFormats[i],queryDescriptions[i],queryExamples[i]);
    }
}

//valida a string recebida
//TODO: verificação mais completa !!!!!!!!!!!!!!
int validQueryInput (char * queryInput) {
    return ((queryInput[0] > 47) && (queryInput[0] < 58)); //temporário
}

//modo interativo de correr queries
// TODO: Error check de inputs para queries no modo interativo
int interactRequests(UserData *userData, DriverData *driverData, RidesData *ridesData) {
    char *strBuffer = malloc(sizeof(char) * LINE_SIZE), // buffer de cada linha lida
         *queryResult = NULL; // pointer para a string resultante de cada querry
     int commandN = 1; // só para debug, pode-se remover depois

    printf("\x1B[1;1H\x1B[2J"); // limpar a tela
    printf("\n<Modo interativo>\n\nEscreva \"help\" na consola para saber todos os comandos disponíveis e o seus formatos\n\nInput:\n");

    while(fgets(strBuffer,LINE_SIZE,stdin)) { // recebe continuamente input
        strBuffer[strcspn(strBuffer, "\n")] = 0; // para remover eventual newline na string de input
        if (!strcmp(strBuffer,"help")) {
            helpCommands();
        }
        else if (validQueryInput(strBuffer)) { // se o input for válido para uma query, calcula-se a resposta
            queryResult = queryAssign(strBuffer,userData,driverData,ridesData,commandN);
            
            if (queryResult == NULL) {// se o resultado da query for NULL
                printf("\nA query não devolveu nenhum resultado :(\n\n");
            }
            else {// se o resultado da querry não for NULL
                printf("\nResultado da query:\n\n%s\n", queryResult);
            }

            commandN ++; // só para debug, pode-se remover depois
            free(queryResult);
        }
        else {
            printf("(!) Input com formato incorreto\nEscreva \"help\" na consola para saber todos os comandos disponíveis e o seus formatos\n\n");
        }
        printf("Input:\n");
    }
    fprintf(stdout, "A sair do modo interativo...\n");
    return 0; // pode ser usado para comandos de erro no futuro talvez?
}

//modo batch de correr queries
int batchRequests(FILE *fp, UserData *userData, DriverData *driverData, RidesData *ridesData) {
    char *strBuffer = malloc(sizeof(char) * LINE_SIZE), // buffer de cada linha lida
         *queryResult = NULL;                          // pointer para a string resultante de cada querry
    ssize_t read;
    size_t len = LINE_SIZE; // para o getline
    int i, commandN = 1, writeRet;

    // lê linhas individualmente até chegar ao fim do ficheiro
    for (i = 0; (read = getline(&strBuffer, &len, fp) != -1); i++, commandN++) {

        queryResult = queryAssign(strBuffer,userData, driverData, ridesData,commandN);
		fflush(stdout);

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



