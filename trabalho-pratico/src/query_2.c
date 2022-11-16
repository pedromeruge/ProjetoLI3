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
    free(currentArrayStruct->ratingChart);
    free(currentArrayStruct->mostRecRideDate);
}

//concatena n linhas com os resultados, para dar return da query_2
static char * strResults(GPtrArray * driverRatingArray, int N, DriverStruct *driverData[]) {
    short int i, driverNumber;
    char * result = malloc(sizeof(char)*STR_BUFF_SIZE*N);
    char * temp = NULL;
    DriverStruct * currentDriver = NULL;
    driverRatingInfo * currentArrayStruct = NULL;
    int arrayLen = driverRatingArray->len;
    for (i=arrayLen-1;i>=arrayLen-N;i--) {
        currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, i);
        driverNumber = currentArrayStruct->driverNumber;
        currentDriver = getDriverPtrByID(driverData,driverNumber);
        temp = malloc(STR_BUFF_SIZE);
        snprintf(temp,STR_BUFF_SIZE,"%012d;%s;%.3f\n",driverNumber,currentDriver->name,*(float *)currentArrayStruct->ratingChart);
        strncat(result,temp,STR_BUFF_SIZE);
    }
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
    float drv1Rating = *(float *)driver1->ratingChart;
    const driverRatingInfo * driver2 = *(driverRatingInfo **) b;
    float drv2Rating = *(float *)driver2->ratingChart;
    float diff = drv1Rating - drv2Rating;
    //printf("drv1Rating: %f drv2Rating: %f diff %f",drv1Rating,drv2Rating,diff);
    gint result = diff * 10000; // suponho três casas decimais de precisão no máximo (10^3~1024)
    if (!result && drv1Rating) {  // previne comparações entre nodos de riders com rating 0 (não apareciam nas rides)
        result = compDates(driver1->mostRecRideDate,driver2->mostRecRideDate);
        if (!result) 
            result = driver1->driverNumber - driver2->driverNumber;
    }
    return result;
}

static GPtrArray * sumRatings (GPtrArray * driverRatingArray, gint arraySize) {
    int i;
    float avgRating = 0;
    unsigned int * ratings = NULL;
    driverRatingInfo * currentArrayStruct = NULL;
    for (i = 0;i<arraySize;i++) {
        //printf("iteration: %d\n",i);
        currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, i);
        if (currentArrayStruct) {
            ratings = (unsigned int *) currentArrayStruct->ratingChart;
            avgRating = 0;
            //printf(">driver: %d, mostrecridedate: %s, ratings: %d %d %d %d %d\n",currentArrayStruct->driverNumber,currentArrayStruct->mostRecRideDate,ratings[0],ratings[1],ratings[2],ratings[3],ratings[4]);
            unsigned int numRatings = 0;
            int j;
            for (j=0;j<5;j++) {
                avgRating += ratings[j]*(j+1);
                numRatings += ratings[j];
            }
            avgRating /= numRatings;
            free(ratings);
            currentArrayStruct->ratingChart = malloc(sizeof(float));
            *(float *)(currentArrayStruct->ratingChart) = avgRating;
            currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, i);
            //printf(">>driver: %d, mostrecridedate: %s, frating: %f\n",currentArrayStruct->driverNumber,currentArrayStruct->mostRecRideDate,*(float *)(currentArrayStruct->ratingChart));
        }
        else{ // caso não haja informação do driver em rides (driver não ativo)
            driverRatingInfo * newStruct = malloc(sizeof(driverRatingInfo));
            newStruct->ratingChart = malloc(sizeof(float));
            * (float *) (newStruct->ratingChart) = 0;
            newStruct->driverNumber = (short int) (i+1);
            newStruct->mostRecRideDate = NULL; // não será usado
            g_ptr_array_index(driverRatingArray, i) = newStruct;
            currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, i); 
            //printf(">EMPTY: driver: %d, mostrecridedate: %s, frating: %f\n",currentArrayStruct->driverNumber,currentArrayStruct->mostRecRideDate,*(float *) (currentArrayStruct->ratingChart));
        }
    }
    return driverRatingArray;
}

//TODO: free dos elementos do g_ptr_array
char * query_2 (char * number, char * trash1, char * trash2, UserData *userData, DriverStruct *driverData[], RidesData *ridesData) {
    int i,driverStatus;
    gint driverNumber;
    gint driversNumber = (gint) DRIVER_ARR_SIZE * SIZE;
    gint elemNumber = (gint) RIDES_ARR_SIZE * SIZE; // numero de elem do array
    driverRatingInfo * newStruct = NULL,// pointer para struct que vai ser guardada em cada pos do array
    * currentArrayStruct = NULL;
    RidesStruct * currentRide = NULL; // pointer para a ride correspondente no ficheiro csv
    DriverStruct * currentDriver = NULL; // pointer para o driver correspondente à ride atual, no ficheiro csv
    GPtrArray * driverRatingArray = g_ptr_array_new(); // novo garraypointer
    g_ptr_array_set_size(driverRatingArray,driversNumber);
    for (i=0;i<elemNumber;i++) {
        currentRide = getRideByID(ridesData->ridesArray,i+1);
        driverNumber = (gint) currentRide->driver; // o array tem posições de 0 a 9999, os driverID vão de 1 a 10000, daí o driverNumber-1
        currentDriver = getDriverPtrByID(driverData,driverNumber);
        driverStatus = currentDriver->status;
        currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, driverNumber-1); 
        // printf("driver status: %d\n", currentDriver->status);
        // printf("00 is array pos %d not occupied: %d, iteration: %d\n",driverNumber-1,currentArrayStruct == 0,i+1);
        //printf(">>>current ride date:%s\n", currentRide->date);
        if (currentArrayStruct == 0 && driverStatus == 1) { // verifica se no local atual ainda n existe informação de um driver, e se este tem o estado ativo
            //printf(">>> entrou no if\n");
            newStruct = newDriverRating(currentRide->date,(short int) driverNumber,currentRide->score_d);
            //printf(">>>newstruct driver saved:%d\n",newStruct->driverNumber);
            g_ptr_array_index(driverRatingArray, driverNumber-1) = newStruct;
            //printf(">>>000 is array pos %d not occupied: %d\n",driverNumber-1,g_array_index(driverRatingArray, driverRatingInfo *, driverNumber-1) == 0);
            // printf(">>>|||arraypos: %d ridedriver:%d\n", driverNumber-1, currentRide->driver);
            currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, driverNumber-1); 
            // printf("driver various ratings: %d %d %d %d %d\n\n",((unsigned int *) currentArrayStruct->ratingChart)[0],((unsigned int *) currentArrayStruct->ratingChart)[1],((unsigned int *) currentArrayStruct->ratingChart)[2],((unsigned int *) currentArrayStruct->ratingChart)[3],((unsigned int *) currentArrayStruct->ratingChart)[4]);
        }
        else if (driverStatus == 1) { // se já existir informação de um driver (de rides prévias); supõe-se que tem estado ativo
            // printf("<<< entrou no else if\n");
            // printf("\nrepetição aqui\n\n");
            ((unsigned int *) currentArrayStruct->ratingChart)[(currentRide->score_d)-1] += 1; // dependendo da avaliação na ride atual, no array ratingChart , incrementa em 1 o número da avalições de valor 1,2,3,4 ou 5. 
            if (compDates(currentRide->date,currentArrayStruct->mostRecRideDate) >= 0) {
                currentArrayStruct->mostRecRideDate = currentRide->date;
            }
            // printf(">>>|||ridedriver:%d\n", currentRide->driver);
            currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, driverNumber-1); 
            // printf("driver various ratings: %d %d %d %d %d; saved ride date: %s\n\n",((unsigned int *) currentArrayStruct->ratingChart)[0],((unsigned int *) currentArrayStruct->ratingChart)[1],((unsigned int *) currentArrayStruct->ratingChart)[2],((unsigned int *) currentArrayStruct->ratingChart)[3],((unsigned int *) currentArrayStruct->ratingChart)[4], currentArrayStruct->mostRecRideDate);
        }
    }
    //função de calcular a média de ratings para cada driver ; insere nodos com rating 0, para riders que não aparecem no ficheiro rides.csv
    driverRatingArray = sumRatings(driverRatingArray,driversNumber);

    g_ptr_array_sort(driverRatingArray,sort_byRatings);

    char * result = strResults(driverRatingArray,number[0]-48, driverData);

    g_ptr_array_set_free_func(driverRatingArray, (GDestroyNotify) freeRidesRating);
    // //para debug
    // for (i=0;i<driversNumber;i++) {
    //     currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, i);
    //     //printf("driver rating:%.5f most recent ride: %s driver number: %d\n", *(float *) currentArrayStruct->ratingChart, currentArrayStruct->mostRecRideDate, currentArrayStruct->driverNumber);
    //     }
   
    // NOTA: query 3 é indentica a função query_2 mas usa outros valores e outra compare func. Fazer um ptr_array com toda a informação e ordená-lo segundo a função dada talvez??
    // NOTA: a maior parte das funções de queries resume-se a "n primeiros" e "valores médios" apenas variando os parâmetros de comparação e o abrangimento dos dados de input. Estas funções mais básicas devem ser capazes de receber diferentes categorias (mais ou menos restritas) de dados e ordená-las com base nos parâmetros dados
    // NOTA: no modo interativo pode ser preciso chamar a mesma função vºarias vezes, guardar as estruturas resultantes da primeira vez que a querry é usada e reutiliza-los das vezes seguintes
    return result;
}
