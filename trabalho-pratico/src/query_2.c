#include "query_2.h"

#define STR_BUFF_SIZE 64

// TODO: Fazer parse de um array com ordem dos users e outro para rides em simultâneo 
//       função listN (void *array, struct *, comparefunc1,comparefunc2,comparefunc3 )

//concatena n linhas com os resultados, para dar return da query_2
static char * strResults(const ridesByDriver * driverRatingArray, int N, DriverData *driverData) {
    short int i, j,driverNumber;
    unsigned char driverStatus;
	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char)); // recebe free em query_requests
    result[0] = '\0';
    char temp[STR_BUFF_SIZE], * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    const driverRatingInfo * currentArrayStruct = NULL;
    short int arrayLen = getridesByDriverArraySize(driverRatingArray);
    for (i=arrayLen, j = N; j>0 && i>0 ;i--) {
        currentArrayStruct = getDriverInfo(driverRatingArray, i);
        driverNumber = getDriverNumber(currentArrayStruct);
        currentDriver = getDriverPtrByID(driverData,driverNumber);
        driverName = getDriverName(currentDriver);
        driverStatus = getDriverStatus(currentDriver);
        if (driverStatus == 1) {
        snprintf(temp,STR_BUFF_SIZE,"%0*d;%s;%.3f\n", 12, driverNumber, driverName, getDriverAvgRating(currentArrayStruct));
        strncat(result,temp,STR_BUFF_SIZE);
        j--;
        }
        free(driverName);
    }
	// printf("result %s\n", result);
    return result;
}

char * query_2 (char * inputStr[], UserData *userData, DriverData * driverData, RidesData *ridesData) {
	int num = atoi(inputStr[0]);
	if (num == 0) return NULL;

    const ridesByDriver * driverRatingArray = getRidesByDriver (ridesData);

    //qSortArray(driverRatingArray,&sort_byRatings); // dá sort todas as vezes, corrigir !!!

    char * result = strResults(driverRatingArray,num, driverData);

    free((ridesByDriver *) driverRatingArray);

    // NOTA: query 3 é indentica a função query_2 mas usa outros valores e outra compare func. Fazer um ptr_array com toda a informação e ordená-lo segundo a função dada talvez??
    // NOTA: a maior parte das funções de queries resume-se a "n primeiros" e "valores médios" apenas variando os parâmetros de comparação e o abrangimento dos dados de input. Estas funções mais básicas devem ser capazes de receber diferentes categorias (mais ou menos restritas) de dados e ordená-las com base nos parâmetros dados
    // NOTA: no modo interativo pode ser preciso chamar a mesma função vºarias vezes, guardar as estruturas resultantes da primeira vez que a querry é usada e reutiliza-los das vezes seguintes
    return result;
}
