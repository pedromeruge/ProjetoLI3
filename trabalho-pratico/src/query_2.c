#include "query_2.h"

#define STR_BUFF_SIZE 64

// TODO: Fazer parse de um array com ordem dos users e outro para rides em simultâneo 
//       função listN (void *array, struct *, comparefunc1,comparefunc2,comparefunc3 )

//concatena n linhas com os resultados, para dar return da query_2
static char * strResults(const ridesByDriver * driverRatingArray, int N, DriverData *driverData) {
    short int i, driverNumber;

	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char));
    result[0] = '\0';
    char temp[STR_BUFF_SIZE], * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    const driverRatingInfo * currentArrayStruct = NULL;
    short int arrayLen = getridesByDriverArraySize(driverRatingArray);
    for (i=arrayLen;i>=arrayLen-N+1;i--) {
        currentArrayStruct = getDriverInfo(driverRatingArray, i);
        driverNumber = getDriverNumber(currentArrayStruct);
        currentDriver = getDriverPtrByID(driverData,driverNumber);
        driverName = getDriverName(currentDriver);
        snprintf(temp,STR_BUFF_SIZE,"%0*d;%s;%.3f\n", 12, driverNumber, driverName, getDriverAvgRating(currentArrayStruct));
        strncat(result,temp,STR_BUFF_SIZE);
        free(driverName);
    }
	// printf("result %s\n", result);
    return result;
}

static gint sort_byRatings (gconstpointer a, gconstpointer b) {
    const driverRatingInfo * driver1 = *(driverRatingInfo **) a;
    const driverRatingInfo * driver2 = *(driverRatingInfo **) b;
    double drv1Rating,
    drv2Rating;
	/*if (driver1 == NULL) drv1Rating = 0; 
    else*/ drv1Rating = getDriverAvgRating(driver1);
	/*if (driver2 == NULL) drv2Rating = 0; 
    else*/ drv2Rating = getDriverAvgRating(driver2);
    double diff = drv1Rating - drv2Rating;	
    gint result = 0;
    if (diff > 0) result = 1;
    else if (diff <0) result = -1;
    else if (!result && drv1Rating) {  // previne comparações entre nodos de riders com rating 0 (não apareciam nas rides)
        result = compDates(getDriverMostRecRideDate(driver1),getDriverMostRecRideDate(driver2));
        if (!result) 
            result = getDriverNumber(driver1) - getDriverNumber(driver2);
    }
    return result;
}

//TODO: free dos elementos do g_ptr_array
// !!!! Teste de meter a struct pública mas return const, para n poder mexer nela

char * query_2 (char * number, char * trash1, char * trash2, UserData *userData, DriverData * driverData, RidesData *ridesData) {
	int num = atoi(number);
	if (num == 0) return NULL;

    const ridesByDriver * driverRatingArray = getRidesByDriver (ridesData);

    //driverRatingInfo * currentArrayStruct = getDriverInfo(driverRatingArray,8899);
    //printf("number: %d, avg-rating: %f, number of rides: %d most-rec-ride: %s\n",getDriverNumber(currentArrayStruct),getDriverAvgRating(currentArrayStruct),getDriverRidesNumber(currentArrayStruct),getDriverMostRecRideDate(currentArrayStruct));
    qSortArray(driverRatingArray,&sort_byRatings);

    char * result = strResults(driverRatingArray,num, driverData);

    //g_ptr_array_free(driverRatingArray, TRUE);

    // NOTA: query 3 é indentica a função query_2 mas usa outros valores e outra compare func. Fazer um ptr_array com toda a informação e ordená-lo segundo a função dada talvez??
    // NOTA: a maior parte das funções de queries resume-se a "n primeiros" e "valores médios" apenas variando os parâmetros de comparação e o abrangimento dos dados de input. Estas funções mais básicas devem ser capazes de receber diferentes categorias (mais ou menos restritas) de dados e ordená-las com base nos parâmetros dados
    // NOTA: no modo interativo pode ser preciso chamar a mesma função vºarias vezes, guardar as estruturas resultantes da primeira vez que a querry é usada e reutiliza-los das vezes seguintes
    return result;
}
