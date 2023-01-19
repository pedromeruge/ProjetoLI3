#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "query_requests.h"
#include "files.h"
#define MAX_QUERY_INPUTS 3
#define MAX_QUERY_OUTPUTS 6
#define TOTAL_QUERIES_NUMBER 9
#define TERMINAL_MAX_N_OF_LINES 20
#define INPUT_STR_BUFF_SIZE 64
#define OUTPUT_STR_BUFF_SIZE 64

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
    // ver se nº de inputs é inferior a 1 ou superior a MAX_QUERY_INPUTS
    // ver se nº de espaços entre argumentos correpsonde ao nº de inputs ?
int validQueryInput (char * queryInput) {
    return ((queryInput[0] > 47) && (queryInput[0] < 58)); //temporário
}

void printQueryOutput(GArray * splitQueryResults, int * segmMaxSizes, int pageN, int numberOfPages, int queryNumber) {
    int i, segmNumber;
    char * currPage = g_array_index(splitQueryResults,char *, pageN-1);
    char * queryDescription[TOTAL_QUERIES_NUMBER] = {"nome;genero;idade;avaliacao_media;numero_viagens;total_gasto",
                                                     "id;nome;avaliacao_media",
                                                     "username;nome;distancia_total",
                                                     "preco_medio",
                                                     "preco_medio",
                                                     "distancia_media",
                                                     "id;nome;avaliacao_media",
                                                     "id_condutor;nome_condutor;username_utilizador;nome_utilizador",
                                                     "id_viagem;data_viagem;distancia;cidade;valor_gorjeta"};

    printf("\n# Query %d output\n",queryNumber);

    // for (i=0;i<MAX_QUERY_OUTPUTS;i++) {
    //     printf("pos %d: %d\n",i, segmMaxSizes[i]);
    // }
    //printf("Formato: %s\n---\nResultado:\n%s\n",queryDescription[queryNumber-1],currPage);
    printf("Formato:\n%s\n---\n", queryDescription[queryNumber-1]);
    while(currPage[0] != '\0') {
      for (i=0, segmNumber=0; currPage[i] != '\n' && currPage[i] != '\0'; i++) {
        if (currPage[i] == ';') {
            currPage = currPage + i+1;
            while (i < segmMaxSizes[segmNumber]) {
            putchar(' ');
            i++;
            }
            i=-1;
            segmNumber++;
            printf(" | ");
        }
        else putchar(currPage[i]);
      }
    currPage = currPage + i+1;
    if (currPage[-1] == '\0') currPage--;
    putchar('\n');
    }

    printf("---\nPage %d-%d\n\n",pageN, numberOfPages);
}


// função que parte o output das queries em várias paginas, para ser mais legível
// a partição em páginas é feita dentro do modo interativo (e não diretamente no output das queries) para não afetar o cálculo do tempo no modo batch, ainda que seja muito mais eficiente no modo batch 
void printTerminalResults(char * queryResult, int queryNumber) {

    //separação da string em segmentos; cálculo dos caractéres necessários para cada parâmetro
    int i, newlineCounter, segmNumber, segmMaxSize, 
        * segmMaxSizes = calloc(MAX_QUERY_OUTPUTS,sizeof(int)); // array que guarda o tamanho da maior palavra, para cada segmento do output
    GArray * splitQueryResults = g_array_new(FALSE, FALSE, sizeof(char *)); // gArray para realocar o array automaticamente

    while (queryResult[0] != '\0') {
        for(i=1,newlineCounter = 0; queryResult[i-1] != '\0' && newlineCounter < TERMINAL_MAX_N_OF_LINES; newlineCounter++, i++) { // para cada "página" de 20 linhas
            for(segmNumber = 0; queryResult[i-1] != '\n'; segmNumber++, i++) { // para cada linha 
                for (segmMaxSize = 0; queryResult[i] != ';' && queryResult[i] != '\n'; segmMaxSize++, i++); // para cada parâmetro de output de 1 linha
                if (segmMaxSize > segmMaxSizes[segmNumber]) segmMaxSizes[segmNumber] = segmMaxSize;
            }
        }
        queryResult[i-2] = '\0';
        g_array_append_val(splitQueryResults,queryResult); // adiciona o segmento de string ao array
        queryResult = queryResult + i-1;
    }
    segmMaxSizes[0] ++; // devido à forma como o loop foi feito (para n ter condições if a mais) inicia-se i=1, daí ter de se incrementar em 1 o valor segmMaxSizes[0]

    int numberOfPages = splitQueryResults->len;
    printf("\x1B[1;1H\x1B[2J"); // limpar a tela
    printQueryOutput(splitQueryResults,segmMaxSizes, 1,numberOfPages, queryNumber);
    if (numberOfPages == 1) return; // se o número de páginas for apenas 1, volta para interactRequests
    
    //apresentação dos segmentos de string por página
    printf("Escolha um número no intervalo (1-%d) para visualizar a respetiva página de output, ou \"continue\" para introduzir inputs para outras queries\n\n",numberOfPages); // só apresentar esta linha quando existem várias páginas
    char * strBuffer = malloc(sizeof(char) * INPUT_STR_BUFF_SIZE); // buffer de cada linha lida // BUFFER MUITO GRANDE VER DEPOIS!!
    while(fgets(strBuffer,INPUT_STR_BUFF_SIZE,stdin)) { // recebe continuamente input
        strBuffer[strcspn(strBuffer, "\n")] = 0;
        printf("\x1B[1;1H\x1B[2J"); // limpar a tela
        if ((i = atoi(strBuffer))>0 && i <= numberOfPages) { // se o input for válido, escreve o output da pagina pedida
            printQueryOutput(splitQueryResults, segmMaxSizes, i, numberOfPages, queryNumber); 
        }
        else if (!strcmp(strBuffer,"continue")) return; // se escrever "continue" volta para interactRequests
        else { // se o input for inválido ,escreve mensagem de erro
            printf("\n(!) Input inválido\n\nEscolha um número no intervalo (1-%d) para visualizar a respetiva página de output, ou \"continue\" para introduzir inputs para outras queries\n\n",numberOfPages);
        }
    }
    free(segmMaxSizes);
    g_array_free(splitQueryResults,TRUE); // dá free das strings, ou não? (isso já é feito na interactRequests)
}

//modo interativo de correr queries
// TODO: Error check de inputs para queries no modo interativo
int interactRequests(UserData *userData, DriverData *driverData, RidesData *ridesData) {
    char *strBuffer = malloc(sizeof(char) * INPUT_STR_BUFF_SIZE), // buffer de cada linha lida
         *queryResult = NULL; // pointer para a string resultante de cada querry
     int commandN = 1; // só para debug, pode-se remover depois

    printf("\x1B[1;1H\x1B[2J"); // limpar a tela
    printf("\n<Modo interativo>\n\nEscreva \"help\" na consola para saber todos os comandos disponíveis e o seus formatos\n\nInput:\n");

    while(fgets(strBuffer,INPUT_STR_BUFF_SIZE,stdin)) { // recebe continuamente input
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
                printTerminalResults(queryResult, (strBuffer[0]-'0'));
            }

            commandN ++; // só para debug, pode-se remover depois
            free(queryResult);
        }
        else {
            printf("\n(!) Input inválido\n\nEscreva \"help\" na consola para saber todos os comandos disponíveis e o seus formatos\n\n");
        }
        printf("Input:\n");
    }
    printf("A sair do modo interativo...\n");
    free(strBuffer);
    return 0; // pode ser usado para comandos de erro no futuro talvez?
}

//modo batch de correr queries
int batchRequests(FILE *fp, UserData *userData, DriverData *driverData, RidesData *ridesData) {
    char *strBuffer = malloc(sizeof(char) * INPUT_STR_BUFF_SIZE), // buffer de cada linha lida
         *queryResult = NULL;                          // pointer para a string resultante de cada querry
    ssize_t read;
    size_t len = INPUT_STR_BUFF_SIZE; // para o getline
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



