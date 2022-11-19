#include <stdio.h>
#include <stdlib.h>
#include "query_2.h"
#include "commonParsing.h"


#define STR_BUFF_SIZE 64

typedef struct {
    void  * ratingChart; // void * porque vai guardar um array de ratings de 1 a 5, e mais tarde é convertido para um valor médio desses ratings
    char * mostRecRideDate;
    short int driverNumber; // talvez meter em int o valor, ocupa menos espaço com char?
} driverRatingInfo;

// TODO: Fazer parse de um array com ordem dos users e outro para rides em simultâneo 
//       função listN (void *array, struct *, comparefunc1,comparefunc2,comparefunc3 )

// função para dar free das structs do ptr_array com informação dos drivers
static void freeRidesRating (void * drivesRating) {
    driverRatingInfo * currentArrayStruct = (driverRatingInfo *) drivesRating;
	//if (currentArrayStruct) {
		free(currentArrayStruct->ratingChart);
		free(currentArrayStruct->mostRecRideDate);
		free(currentArrayStruct);
	//}
}

//concatena n linhas com os resultados, para dar return da query_2
static char * strResults(GPtrArray * driverRatingArray, int N, DriverData *driverData) {
    short int i, driverNumber;
	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char));
    result[0] = '\0';
    char temp[STR_BUFF_SIZE], * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    driverRatingInfo * currentArrayStruct = NULL;
    int arrayLen = driverRatingArray->len;
    for (i=arrayLen-1;i>=arrayLen-N;i--) {
        currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, i);
        driverNumber = currentArrayStruct->driverNumber;
        currentDriver = getDriverPtrByID(driverData,driverNumber);
        driverName = getDriverName(currentDriver);
        snprintf(temp,STR_BUFF_SIZE,"%0*d;%s;%.3f\n", 12, driverNumber, driverName, *(double *)currentArrayStruct->ratingChart);
        strncat(result,temp,STR_BUFF_SIZE);
        free(driverName);
    }
	// printf("result %s\n", result);
    return result;
} 


static driverRatingInfo * newDriverRating (char * rideDate, short int driverNumber, short int rating) {
    driverRatingInfo * new = malloc(sizeof(driverRatingInfo));
    new->ratingChart = calloc(5,sizeof(unsigned int)); // inicializa todas as avaliações como zero
    ((unsigned int *) new->ratingChart)[rating-1] ++;
    new->mostRecRideDate = rideDate; // devo copiar a string para a nova struct, para não haver dependências no futuro??
    new->driverNumber = driverNumber;
    return new;
}

static gint sort_byRatings (gconstpointer a, gconstpointer b) {
    const driverRatingInfo * driver1 = *(driverRatingInfo **) a;
    const driverRatingInfo * driver2 = *(driverRatingInfo **) b;
    double drv1Rating,
    drv2Rating;
	/*if (driver1 == NULL) drv1Rating = 0; 
    else*/ drv1Rating = *(double *)driver1->ratingChart;
	/*if (driver2 == NULL) drv2Rating = 0; 
    else*/ drv2Rating = *(double *)driver2->ratingChart;
    double diff = drv1Rating - drv2Rating;	
    gint result = 0;
    if (diff > 0) result = 1;
    else if (diff <0) result = -1;
    else if (!result && drv1Rating) {  // previne comparações entre nodos de riders com rating 0 (não apareciam nas rides)
        result = compDates(driver1->mostRecRideDate,driver2->mostRecRideDate);
        if (!result) 
            result = driver1->driverNumber - driver2->driverNumber;
    }
    return result;
}

static GPtrArray * sumRatings (GPtrArray * driverRatingArray) {
    int i;
    double avgRating = 0;
    unsigned int * ratings = NULL,
                   numRatings = 0;
    driverRatingInfo * currentArrayStruct = NULL;
    gint arraySize = driverRatingArray->len;
    for (i = 0;i<arraySize;i++) {
        currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, i);
        if (currentArrayStruct) {
            ratings = (unsigned int *) currentArrayStruct->ratingChart;
            avgRating = 0;
            numRatings = 0;
            int j;
            for (j=0;j<5;j++) {
                avgRating += ratings[j]*(j+1);
                numRatings += ratings[j];
            }
            avgRating /= numRatings;
            free(ratings);
            currentArrayStruct->ratingChart = malloc(sizeof(double));
            *(double *)(currentArrayStruct->ratingChart) = avgRating;
        }
		 else{ // caso não haja informação do driver em rides (driver não ativo)
            driverRatingInfo * newStruct = malloc(sizeof(driverRatingInfo));
            newStruct->ratingChart = malloc(sizeof(double));
            * (double *) (newStruct->ratingChart) = 0;
            newStruct->driverNumber = (short int) (i+1);
            newStruct->mostRecRideDate = NULL; // não será usado
            g_ptr_array_index(driverRatingArray, i) = newStruct;
        }
    }
    return driverRatingArray;
}

//TODO: free dos elementos do g_ptr_array
char * query_2 (char * number, char * trash1, char * trash2, UserData *userData, DriverData * driverData, RidesData *ridesData) {
	int num = atoi(number);
	if (num == 0) return NULL;
    int i,driverStatus = 0;
    short int driverNumber = 0, scoreD = 0;
    int driversNumber = DRIVER_ARR_SIZE * SIZE;
    int elemNumber = RIDES_ARR_SIZE * SIZE; // numero de elem do array
    char * rideDate = NULL;
    driverRatingInfo * newStruct = NULL,// pointer para struct que vai ser guardada em cada pos do array
    * currentArrayStruct = NULL;
    RidesStruct * currentRide = NULL; // pointer para a ride correspondente no ficheiro csv
    DriverStruct * currentDriver = NULL; // pointer para o driver correspondente à ride atual, no ficheiro csv
    GPtrArray * driverRatingArray = g_ptr_array_new_full(driversNumber,(GDestroyNotify) freeRidesRating); // novo garraypointer

    // o tamanho do array tem de ser definido, apesar de ja ter sido alocado o espaço para o tamanho necessário
    // o array tem de ser inicializado a NULL para todos os pointers, feito por g_ptr_array_set_size
	g_ptr_array_set_size(driverRatingArray,driversNumber);

    for (i=0;i<elemNumber;i++) {
        currentRide = getRidePtrByID(ridesData, i+1);
        driverNumber = getRideDriver(currentRide); // o array tem posições de 0 a 9999, os driverID vão de 1 a 10000, daí o driverNumber-1
        currentDriver = getDriverPtrByID(driverData, driverNumber);
        driverStatus = getDriverStatus(currentDriver);
        rideDate = getRideDate(currentRide);
        scoreD = getRideScore_d(currentRide);
        currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, driverNumber-1);
        if (currentArrayStruct == NULL && driverStatus == 1) { // verifica se no local atual ainda n existe informação de um driver, e se este tem o estado ativo
            newStruct = newDriverRating(rideDate,(short int) driverNumber, scoreD);
            g_ptr_array_index(driverRatingArray, driverNumber-1) = newStruct;
        }
        else if (driverStatus == 1) { // se já existir informação de um driver (de rides prévias); supõe-se que tem estado ativo
            ((unsigned int *) currentArrayStruct->ratingChart)[scoreD-1] += 1; // dependendo da avaliação na ride atual, no array ratingChart , incrementa em 1 o número da avalições de valor 1,2,3,4 ou 5. 
            if (compDates(rideDate,currentArrayStruct->mostRecRideDate) >= 0) {
                free(currentArrayStruct->mostRecRideDate);
                currentArrayStruct->mostRecRideDate = rideDate;
            }
            else free(rideDate);
        }
        else free(rideDate); // a data da ride correspondia a um driver inativo, a string recebeu malloc noutro módulo
    }
    //função de calcular a média de ratings para cada driver ; insere nodos com rating 0, para riders que não aparecem no ficheiro rides.csv
    driverRatingArray = sumRatings(driverRatingArray);

    g_ptr_array_sort(driverRatingArray,sort_byRatings);

    char * result = strResults(driverRatingArray,num, driverData);

    g_ptr_array_free(driverRatingArray, TRUE);

    // NOTA: query 3 é indentica a função query_2 mas usa outros valores e outra compare func. Fazer um ptr_array com toda a informação e ordená-lo segundo a função dada talvez??
    // NOTA: a maior parte das funções de queries resume-se a "n primeiros" e "valores médios" apenas variando os parâmetros de comparação e o abrangimento dos dados de input. Estas funções mais básicas devem ser capazes de receber diferentes categorias (mais ou menos restritas) de dados e ordená-las com base nos parâmetros dados
    // NOTA: no modo interativo pode ser preciso chamar a mesma função vºarias vezes, guardar as estruturas resultantes da primeira vez que a querry é usada e reutiliza-los das vezes seguintes
    return result;
}
