#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <locale.h> // posso usar?? permite caractéres em UTF-8 no modo interativo
#include "query_requests.h"
#include "files.h"
#define MAX_QUERY_INPUTS 3
#define MAX_QUERY_OUTPUTS 6
#define TOTAL_QUERIES_NUMBER 9
#define TERMINAL_MAX_N_OF_LINES 20
#define INPUT_STR_BUFF_SIZE 64
#define OUTPUT_STR_BUFF_SIZE 64

void printResultInPages(char * queryResult, int queryNumber, WINDOW * windows[]);

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
	// ***** printf("%d:%d ", commandN,(*tempsegstr[0]) - 49 + 1);

	// int j;
    // for (j = 1;j <= MAX_QUERY_INPUTS && tempsegstr[j]; j++)
    //     printf(" <%.16s>",tempsegstr[j]);
    // putchar('\n');
    //### 

    return (queryList[(*tempsegstr[0]) - 49](tempsegstr+1, userData, driverData, ridesData)); // -48 para dar o numero correto, -1 para a query 1 dar no lugar 0
}

//valida a string recebida
//TODO: verificação mais completa !!!!!!!!!!!!!!
    // ver se nº de inputs é inferior a 1 ou superior a MAX_QUERY_INPUTS
    // ver se nº de espaços entre argumentos correpsonde ao nº de inputs ?
int validQueryInput (char * queryInput) {
    return ((queryInput[0] > 47) && (queryInput[0] < 58)); //temporário
}

// apaga a linha atual na janela dada e escreve a str recebida
void refreshWindow (WINDOW * window, char * str) {
    werase(window);
    attron(A_BLINK);
    mvwaddstr(window,0,0,str);
    attroff(A_BLINK);
    wrefresh(window);
}

void printHelpCommands(GArray * splitQueryResults, int pageN, int numberOfPages, WINDOW * windows[]) {
    WINDOW * output = windows[2];
    char * currPage = g_array_index(splitQueryResults,char *, pageN-1);  // string da página atual (página pageN)
    wclear(output);
    wprintw(output,"Queries disponíveis:");
    mvwprintw(output,2,0,"%s",currPage);
    mvwprintw(output,getmaxy(output)-2,0,"---\nPage %d-%d",pageN, numberOfPages);
    wrefresh(output);
}

//escreve o output da query no ecrã, acrescentando espaços e linhas para organizar as palavras em colunas mais legíveis
void printQueryStr(char * str, int * paramMaxSizes, WINDOW * window) {
    int i, segmNumber, currentX = getcury(window), currentY;
    while(str[0] != '\0') {
        for (i=0, segmNumber=0, currentY = -1; str[i] != '\n' && str[i] != '\0'; i++) { 
            if (str[i] == ';') {
                str = str + i+1;
                currentY += paramMaxSizes[segmNumber] + 1; // + 1, simula o incremento que é feito no pointer digital, quando se usa addch;
                wmove(window,currentX,currentY);
                i=-1;
                segmNumber++;
                waddch(window,'|');
            }
            else wprintw(window,"%c",str[i]); // printw permite UTF-8, enquanto addch não
       }
        str = str + i + 1;
        currentX++; // desce uma linha o cursor
        if (str[-1] == '\0') str--; // no caso de paragem, a string aponta para o '\0'
        waddch(window,'\n');
     }
    
    //tentativa anterior
    // for(i=0, segmNumber = 0; str[i] != '\0'; segmNumber++, i++) { 
    //     str = str + i;
    //     for (i = 0, currentParamMax = paramMaxSizes[segmNumber]; str[i] != ';' && str[i] != '\0'; i++); // detetar o fim de um parâmetro, ou o fim de uma linha
    //     str[i] = '\0'; // substitui o ';' ou '\n' por '\0' para poder imprimir esse segmento de string apenas // não fuciona para strings alocadas na staack como as queryDescriptions!!
    //     waddstr(window,str);
    //     for(temp = i; temp < currentParamMax; waddch(window,' '),temp ++); // adicionar espaços até chegar ao fim da coluna
    //     waddch(window,'|');
    // }
    // wdelch(window); wdelch(window);
}

// função que imprime os conteúdos de uma página de output de uma query, partindo os parâmetros de cada linha da string em diferentes colunas
void printQueryOutput(GArray * splitQueryResults,int pageN, int * paramMaxSizes, int numberOfPages, int queryNumber, WINDOW * windows[]) {
    // o print das queries inclui uma descrição do que é cada parâmetro de output
    char * queryDescriptions[TOTAL_QUERIES_NUMBER] = {"nome;genero;idade;avaliacao_media;numero_viagens;total_gasto",
                                                     "id;nome;avaliacao_media",
                                                     "username;nome;distancia_total",
                                                     "preco_medio",
                                                     "preco_medio",
                                                     "distancia_media",
                                                     "id;nome;avaliacao_media",
                                                     "id_condutor;nome_condutor;username_utilizador;nome_utilizador",
                                                     "id_viagem;data_viagem;distancia;cidade;valor_gorjeta"}; 
                                                     
    int i, segmNumber, totalChar;
    WINDOW * output = windows[2];
    char * currPage = g_array_index(splitQueryResults,char *, pageN-1),  // string da página atual (página pageN)
         * currentDescription = queryDescriptions[queryNumber-1];

    // calcular se alguma descrição ocupa mais caractéres que o max de caractéres do respetivo parâmetro do output
    for(i=0,segmNumber = 0, totalChar = 0; currentDescription[i] != '\0'; segmNumber++, i++) { 
        currentDescription = currentDescription + i;
        for (i = 0; currentDescription[i] != ';' && currentDescription[i] != '\0'; i++);
        if (i > paramMaxSizes[segmNumber]) paramMaxSizes[segmNumber] = i;
        totalChar += paramMaxSizes[segmNumber];
        if (currentDescription[i] == '\0') i--;
    }

    //calcular se existe espaço no terminal para expandir o tamanho de cada coluna, para a tabela ficar proporcional ao tamanho do terminal
    int spareChar = getmaxx(output) - totalChar - segmNumber + 2; // no fim do loop anterior, segmNumber guarda o nº de parâmetros na query atual, totalChar guarda o nº de char numa linha, ocupado pelo output
    spareChar /= segmNumber; // spareChar representa o nº de caractéres adicional que se pode acrescentar a cada tabela
    
    wclear(output);
    for(i = 0; i < segmNumber; i++) {
        wprintw(output,"%d;", paramMaxSizes[i]);
    }
    waddch(output,'\n');
    if (spareChar > 0) for(i = 0; i < segmNumber; paramMaxSizes[i] += spareChar, i++);

    for(i = 0; i < segmNumber; i++) {
        wprintw(output,"%d;", paramMaxSizes[i]);
    }

    wprintw(output,"segmNumber:%d, spareChar: %d, # Query %d output\n---\n",segmNumber, spareChar, queryNumber);

    printQueryStr(queryDescriptions[queryNumber-1],paramMaxSizes, output); // print da descrição da query
    printQueryStr(currPage,paramMaxSizes,output); // print do output da query
    //wprintw(output,"%s\n",currPage);

    mvwprintw(output,getmaxy(output)-2,0,"---\nPage %d-%d",pageN, numberOfPages);
    wrefresh(output);
}

// parte a string em conjuntos de n linhas, colocando-as no GArray
// a sepração de strings garante que informações sobre uma query específica não são separados em páginas diferentes (para definir uma secção que não deve ser separada em páginas diferentes, ela tem de acabar com 2 '\n' seguidos)
GArray * splitString (char * str, int numberOfLines, int numberOfColumns) {
    int i, counter, lineTracker, groupTracker; // lineTracker contabiliza linhas extras, quando a frase não cabe numa linha do terminal, groupTracker garante que não aparece partes de informação de uma query em páginas diferentes
    GArray * splitStrResults = g_array_new(FALSE, FALSE, sizeof(char *));
    while (str[0] != '\0') {
        for(i = 0, lineTracker = 0, groupTracker = 0, counter = 0; str[i] != '\0' && counter < numberOfLines; lineTracker++,i++) {
            if (str[i] == '\n' || lineTracker == numberOfColumns) {
                counter++;
                lineTracker = 0;
            }
        }
        if(str[i-2] != '\n') { // quando a string ia partir a meio a informação de uma query, essa string é passada toda para a próxima página
            while (i!=2 && !(str[i-2] == '\n' && str[i-1] == '\n')) i--;

        }
        str[i-1] = '\0';
        if(str[i] == '\n') i++; // nos casos em que temos duas newlines seguidas (fim da descrição de uma querry), parando na primeira, salta-se a segunda para não passar para a próxima página e desformatar
        g_array_append_val(splitStrResults,str);
        str = str + i;
        }
    return splitStrResults;
}

// função que parte o output das queries em várias paginas, para ser mais legível
// simultaneamente, preencha cada posição do array de ints com o nº max de caractéres ocupados pelas palavras do respetivo parâmetro. Ex: tendo 2 linhas com 2 param "asg;gasead" e "asfj;agas", o array devolvido seria [4,6];
//supõe-se que cada linha de input (linhas acabam em \n) cabe numa linha do terminal
// NOTA: se recebesse o nº de linhas da str, originalmente, a conta ficava mais simples, evita-se o GArray! Bastava calcular nº de '\n' na str original e somar 1 para a última linha -> não basta receber o nº de linhas, porque pode variar se uma linha inteira não couber numa janela!

GArray * splitStringMaxParam(char * queryResult, int numberOfLines, int * * arrayToFill) {
    int i, newlineCounter, segmNumber, segmMaxSize,
        * paramMaxSizes = calloc(MAX_QUERY_OUTPUTS,sizeof(int)); // array que guarda o tamanho da maior palavra, para cada segmento do output
    GArray * splitQueryResults = g_array_new(FALSE, FALSE, sizeof(char *)); // gArray para realocar o array automaticamente

    while (queryResult[0] != '\0') {
        for(i=1,newlineCounter = 0; queryResult[i-1] != '\0' && newlineCounter < numberOfLines; newlineCounter++, i++) { // para cada "página" de 20 linhas
            for(segmNumber = 0; queryResult[i-1] != '\n'; segmNumber++, i++) { // para cada linha 
                for (segmMaxSize = 0; queryResult[i] != ';' && queryResult[i] != '\n'; segmMaxSize++, i++); // para cada parâmetro de output de 1 linha
                if (segmMaxSize > paramMaxSizes[segmNumber]) paramMaxSizes[segmNumber] = segmMaxSize;
            }
        }
        queryResult[i-2] = '\0';
        g_array_append_val(splitQueryResults,queryResult); // adiciona o segmento de string ao array
        queryResult = queryResult + i-1;
    }
    paramMaxSizes[0] ++; // devido à forma como o loop foi feito (para n ter condições if a mais) inicia-se i=1, daí ter de se incrementar em 1 o valor segmMaxSizes[0]
    (*arrayToFill) = paramMaxSizes;
    return splitQueryResults;
}

// menu interativo para ler uma string em várias páginas; verifica inputs errados e devolve mensagens de erro
// se queryNumber recebido == -1, o output de cada página não é partido em colunas (usado no menu dos "help" commands)
// NOTA: a partição em páginas é feita dentro do modo interativo (e não diretamente no output das queries) para não afetar o cálculo do tempo no modo batch, ainda que seja mais eficiente no modo batch 
void printResultInPages(char * queryResult, int queryNumber, WINDOW * windows[]) {

    WINDOW //* tabs = windows[0],
    //* outputBorder = windows[1],
    * output = windows[2],
    //* inputBorder = windows[3],
    * input = windows[4];

    //separação da string em segmentos, um para cada página; cálculo dos caractéres necessários para cada parâmetro (apenas quando escreve output de uma query)
    int * segmMaxSizes = NULL;
    GArray * splitQueryResults = (queryNumber != -1) ? splitStringMaxParam(queryResult,getmaxy(output)-5, &segmMaxSizes) 
                                                     : splitString(queryResult,getmaxy(output)-4,getmaxx(output));

    //apresentação da primeira página (acontece sempre)
    int numberOfPages = splitQueryResults->len;
    (queryNumber != -1) ? printQueryOutput(splitQueryResults, 1, segmMaxSizes, numberOfPages, queryNumber, windows) 
                        : printHelpCommands(splitQueryResults,1, numberOfPages, windows);

    if (numberOfPages == 1) return; // se o número de páginas for apenas 1, volta para interactRequests

    // menu de apresentação de outras páginas (quando há 2 ou mais páginas)
    wprintw(output,"Escolha um número no intervalo (1-%d) para visualizar a respetiva página de output, ou \"continue\" para introduzir inputs para outras queries",numberOfPages); // só apresentar esta linha quando existem várias páginas

    refreshWindow(input,"Page:");

    char * strBuffer = malloc(sizeof(char) * INPUT_STR_BUFF_SIZE); // buffer de cada linha lida
    int i;

    while (wgetnstr(input,strBuffer, INPUT_STR_BUFF_SIZE) == OK) {// recebe continuamente input
        wclear(output);
        if ((i = atoi(strBuffer))>0 && i <= numberOfPages) { // se o input for válido, escreve o output da pagina pedida
            (queryNumber != -1) ? printQueryOutput(splitQueryResults, i, segmMaxSizes, numberOfPages, queryNumber, windows) 
                                : printHelpCommands(splitQueryResults, i ,numberOfPages, windows);
        }
        else if (!strcmp(strBuffer,"continue")) {
            werase(output);
            wrefresh(output);
            return; // se escrever "continue" volta para interactRequests
        }
        else { // se o input for inválido ,escreve mensagem de erro
            wprintw(output,"(!) Input inválido\n\n Opções de input:\n\n - Número no intervalo (1-%d) para ver a respetiva página de output\n   Exemplo: \"2\" -> visualizar a página 2\n\n - Palavra \"continue\" para poder introduzir inputs para outras queries",numberOfPages);
            wrefresh(output);
        }
        // recriar linha de input limpa
        refreshWindow(input,"Page:");
        //manter linha de output, para ver o resultado, enquanto não adiciona novo input        
    }
    free(segmMaxSizes);
    g_array_free(splitQueryResults,TRUE); // dá free das strings, ou não? (isso já é feito na interactRequests)
}

// print da tela inicial
void printBeginScreen(WINDOW * beginWindow, int maxX, int maxY) {
    char * initMsg[] = {"<Modo interativo>","Pressione qualquer tecla para iniciar"};
    curs_set(0); // meter o cursor invisível na tela inicial
    attron(A_BOLD);
    mvaddstr(maxX/2, (maxY - strlen(initMsg[0]))/2, initMsg[0]);
    attron(A_BLINK);
    mvaddstr(maxX/2 + 1, (maxY - strlen(initMsg[1]))/2, initMsg[1]);
    attroff(A_BOLD | A_BLINK);
    wborder(stdscr,'#','#','#','#','#','#','#','#');
    refresh();
    getch(); // para a tela até receber qualquer input
    curs_set(1); // meter o cursor visível de novo
    clear();
}

//ações para quando o user escreve "help", no modo interativo
void helpCommands (WINDOW * windows[]) {
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

    WINDOW //* tabs = windows[0],
    //* outputBorder = windows[1],
    * output = windows[2];
    //* inputBorder = windows[3],
    //* input = windows[4];

    werase(output);

    //construir uma string com o texto que aparece na tab "help"
    char helpInstructions[TOTAL_QUERIES_NUMBER * 250] = "", buffer[250]; // 250 é o número de caractéres que a explicação de cada query ocupa aproximadamente

    for (i = 0; i < TOTAL_QUERIES_NUMBER; i++) {
        sprintf(buffer,"Q%d|\nFormato: \"%s\"\nDescrição: %s\nExemplo: \"%s\"\n\n", i+1, queryFormats[i],queryDescriptions[i],queryExamples[i]);
        strcat(helpInstructions,buffer);
    }
    //waddstr(output,helpInstructions);
    printResultInPages(helpInstructions,-1,windows); // apresentação do output em páginas

    wrefresh(output);
}

//modo interativo de correr queries
// TODO: Error check de inputs para queries no modo interativo
int interactRequests(UserData *userData, DriverData *driverData, RidesData *ridesData) {
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC,"C");
    initscr(); //iniciar o ncurses

    int maxX,maxY;
    getmaxyx(stdscr,maxX, maxY); // receber o nº de de row and columns
    if (maxX< 24 || maxY < 80) {
        fprintf(stderr,"Screen size is too small! Run the program again with standard terminal size (80,24) or bigger\n");
        return 1;
    }

    //janela inicial; ao pressionar uma tecla salta para a página de input nas queries
    printBeginScreen(stdscr,maxX,maxY);

    //janela das queries
    WINDOW * tabs = newwin(3,maxY,0,0), 
    * outputBorder = newwin(maxX-6,maxY,3,0), // borda da janela "output" (assim não é afetada por newlines escritas na subjanela)
    * output = newwin(maxX-8,maxY-2,4,1),
    * inputBorder = newwin(3,maxY,maxX-3,0),
    * input = newwin(1,maxY-2,maxX-2,1),
    * allWindows[] = {tabs,outputBorder,output,inputBorder,input};
    refresh(); // refresh de "todas" as janelas

    mvwaddstr(tabs,1,1," Queries   Help"); // printf sem input
    mvwaddstr(input,0,0,"Input:");
    
    box(tabs,0,0);
    box(outputBorder,0,0);
    //box(output,0,0);
    box(inputBorder,0,0);

    wrefresh(outputBorder); // refresh da borda tem que vir antes, senão limpa tudo o que está no "outputs" !!
    //wrefresh(output);
    wrefresh(tabs);
    wrefresh(inputBorder);

    char *strBuffer = malloc(sizeof(char) * INPUT_STR_BUFF_SIZE), // buffer de cada linha lida
         *queryResult = NULL; // pointer para a string resultante de cada querry
    int commandN = 1; // só para debug, pode-se remover depois

    while (wgetnstr(input,strBuffer, INPUT_STR_BUFF_SIZE) == OK) { // receber input no terminal
        if (!strcmp(strBuffer,"exit")) { // se quiser sair do programa
            break;
        }
        else if (!strcmp(strBuffer,"help")) { // se quiser os comandos de ajuda
            helpCommands(allWindows);
        }
        else if (validQueryInput(strBuffer)) { // se o input for válido para uma query, calcula-se a resposta
            queryResult = queryAssign(strBuffer,userData,driverData,ridesData,commandN);
            
            if (queryResult == NULL) {// se o resultado da query for NULL
                waddstr(output,"A query não devolveu nenhum resultado :(");
                wrefresh(output);
            }
            else {// se o resultado da querry não for NULL
                printResultInPages(queryResult, atoi(strBuffer), allWindows); // atoi para no espaço, logo só lê o número da query
            }

            commandN ++; // só para debug, pode-se remover depois
            free(queryResult);
        }
        else {
            waddstr(output,"(!) Input inválido\n\nEscreva \"help\" na consola para saber todos os comandos disponíveis e o seus formatos\n\n");
            wrefresh(output);
        }
        // recriar linha de input com apenas a string "Input:"
        refreshWindow(input,"Input:");
        //manter linha de output, para ver o resultado, enquanto não adiciona novo input
        //removi debug da queryAssign, estava a afetar a window de input !!, quando reformular a validação de input, tirar de coentário !
    }
    free(strBuffer);
    endwin();
    return 0;
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



