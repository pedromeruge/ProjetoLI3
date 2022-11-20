#include <stdio.h>
#include <stdlib.h>
#include "ridesByDriver.h"

static void freeRidesRating (void * drivesRating) {
    driverRatingInfo * currentArrayStruct = (driverRatingInfo *) drivesRating;
    free(currentArrayStruct->ratingChart);
    free(currentArrayStruct->mostRecRideDate);
    free(currentArrayStruct);
}

//adicionar dados de novas ride a struct já existentes 
static driverRatingInfo * appendDriverInfo (driverRatingInfo * currentArrayStruct, RidesStruct * currentRide) {

    short int scoreD = getRideScore_d(currentRide),
            rideDist = getRideDistance(currentRide);
    char * rideDate = getRideDate(currentRide);
    float tip = getRideTip(currentRide);

   currentArrayStruct->moneyData += tip;

    unsigned int * ratings = (unsigned int *) currentArrayStruct->ratingChart;

    ratings[scoreD-1] ++; // dependendo da avaliação na ride atual, no array ratingChart , incrementa em 1 o número da avalições de valor 1,2,3,4 ou 5. 
    ratings[5] += rideDist;
    if (compDates(rideDate,currentArrayStruct->mostRecRideDate) >= 0) {
        free(currentArrayStruct->mostRecRideDate);
        currentArrayStruct->mostRecRideDate = rideDate;
    }
    else free(rideDate);

    return (currentArrayStruct);
}

//criar uma nova struct com informação de um rider ( na primeira vez que aparece nas rides)
static driverRatingInfo * newDriverInfo (RidesStruct * currentRide, DriverStruct * currentDriver) {

    driverRatingInfo * new = malloc(sizeof(driverRatingInfo));

    unsigned char carType = getDriverCar(currentDriver);
    short int driverNumber = getRideDriver(currentRide),
              scoreD = getRideScore_d(currentRide),
              rideDist = getRideDistance(currentRide);
    char * rideDate = getRideDate(currentRide);
    float tip = getRideTip(currentRide);

    new->ratingChart = calloc(6,sizeof(unsigned int));
    unsigned int * ratings = (unsigned int *) new->ratingChart;
    ratings[scoreD-1] ++;
    ratings[5] = rideDist;
    new->moneyData = tip;
    new->mostRecRideDate = rideDate; // devo copiar a string para a nova struct, para não haver dependências no futuro??
    new->driverNumber = driverNumber;
    new->ridesNumber = carType; // temporariamente guardo o tipo de carro nesta variável, porque não é utilizada até ao final do ciclo do addRides
    return new;
}

//cria uma nova struct com o número do driver e o resto da informação nula ( para drivers que não estão ativos/não tenham rides para poder comparar )
driverRatingInfo * newOpaqueDriverInfo (int driverNumber) {
    driverRatingInfo * newStruct = malloc(sizeof(driverRatingInfo));
    newStruct->ratingChart = malloc(sizeof(double)); // precisa de ser double, para na função de comparação não dar problemas
    * (double *) (newStruct->ratingChart) = 0;
    newStruct->driverNumber = (short int) driverNumber;
    newStruct->mostRecRideDate = NULL; // não será usado
    return(newStruct);
}

driverRatingInfo * sumValues (driverRatingInfo * currentArrayStruct, float * carTypeRates) {
    unsigned int * ratings = (unsigned int *) currentArrayStruct->ratingChart,
                   numRides = 0;
    double avgRating = 0;
    float total_auferido = currentArrayStruct->moneyData; // começa com o total de tips
    unsigned char carType = currentArrayStruct->ridesNumber;
    int j;
    for (j=0;j<5;j++) {
        avgRating += ratings[j]*(j+1);
        numRides += ratings[j];
    }
    avgRating /= numRides;
    total_auferido += carTypeRates[((int) carType) * 2] + carTypeRates[((int) carType * 2) + 1];
    free(ratings);
    currentArrayStruct->ridesNumber = numRides;
    currentArrayStruct->moneyData = total_auferido;

    currentArrayStruct->ratingChart = malloc(sizeof(double));
    *(double *)(currentArrayStruct->ratingChart) = avgRating;
    return (currentArrayStruct);
}

// soma valores internos adquiridos ao longo das rides, para obter avaliações médias, distância percorrida, total_auferido
static GPtrArray * getFinalValues (GPtrArray * driverRatingArray) {
    int i;
    driverRatingInfo * currentArrayStruct = NULL,
                     * newStruct = NULL;
    gint arraySize = driverRatingArray->len;
    float carTypeRates[6] = {3.25,0.62,4,0.79,5.20,0.94}; // tarifas para cada tipo de carro; cada 2 números consecutivos são o preço base, e a segunda a taxa por quilómetro
    for (i = 0;i<arraySize;i++) {
        currentArrayStruct = (driverRatingInfo * ) g_ptr_array_index(driverRatingArray, i);
        if (currentArrayStruct) {
            sumValues(currentArrayStruct,carTypeRates);
        }
		 else{ // caso não haja informação do driver em rides (driver não ativo)
            newStruct = newOpaqueDriverInfo(i+1);
            g_ptr_array_index(driverRatingArray, i) = newStruct;
        }
    }
    return driverRatingArray;
}

//acrescenta a infromação de uma ride ao array de drivers
static GPtrArray * addRides (GPtrArray * driverRatingArray, DriverData * driverData, RidesData *ridesData) {
    int elemNumber = RIDES_ARR_SIZE * SIZE,
        driverStatus = 0, i;
    short int driverNumber = 0;
    driverRatingInfo * newStruct = NULL,// pointer para struct que vai ser guardada em cada pos do array
                     * currentArrayStruct = NULL;
    RidesStruct * currentRide = NULL; // pointer para a ride correspondente no ficheiro csv
    DriverStruct * currentDriver = NULL;
    for (i=0;i<elemNumber;i++) {
        currentRide = getRidePtrByID(ridesData, i+1);
        driverNumber = getRideDriver(currentRide); // o array tem posições de 0 a 9999, os driverID vão de 1 a 10000, daí o driverNumber-1
        currentDriver = getDriverPtrByID(driverData, driverNumber);
        driverStatus = getDriverStatus(currentDriver);
        currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, driverNumber-1);

        if (currentArrayStruct == NULL && driverStatus == 1) { // verifica se no local atual ainda n existe informação de um driver, e se este tem o estado ativo
            newStruct = newDriverInfo(currentRide,currentDriver);
            g_ptr_array_index(driverRatingArray, driverNumber-1) = newStruct;
        }
        else if (driverStatus == 1) { // se já existir informação de um driver (de rides prévias); supõe-se que tem estado ativo
            appendDriverInfo (currentArrayStruct,currentRide);
        }
    }
    return (driverRatingArray);
}

GPtrArray * getRidesByDriver (DriverData * driverData, RidesData *ridesData) {
    int driversNumber = DRIVER_ARR_SIZE * SIZE;

    GPtrArray * driverRatingArray = g_ptr_array_new_full(driversNumber,(GDestroyNotify) freeRidesRating); // novo garraypointer
    // o tamanho do array tem de ser definido, apesar de ja ter sido alocado o espaço para o tamanho necessário
    // o array tem de ser inicializado a NULL para todos os pointers, feito por g_ptr_array_set_size
    g_ptr_array_set_size(driverRatingArray,driversNumber);

    driverRatingArray = addRides(driverRatingArray, driverData,ridesData);

    //função de calcular a média de ratings, o total_auferido, distancia total, a partir dos dados recolhidos anteriormente para cada driver; 
    // também insere nodos com rating 0, para riders que não aparecem no ficheiro rides.csv (previne erros em funções de sort que possam ser usadas noutros módulos)
    driverRatingArray = getFinalValues(driverRatingArray);

    driverRatingInfo * currentArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverRatingArray, 8898);
    double avgRating = *(double *) currentArrayStruct->ratingChart;
    printf("avgrat: %lf, total_auferido: %f, recdate: %s, ridesNumber: %d, driverNumber: %d\n",avgRating,currentArrayStruct->moneyData,currentArrayStruct->mostRecRideDate,currentArrayStruct->ridesNumber,currentArrayStruct->driverNumber);
    return driverRatingArray;
}
