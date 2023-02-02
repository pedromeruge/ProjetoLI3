#include "query_common_funcs.h"

#define STR_BUFF_SIZE 64

char * topN(const RidesData * ridesData, const DriverData * driverData, int N, const void * funcArgument, topNfunc getDriverRating) {
    int i, j, driverNumber, arrayLen = getDriversRatingArraySize(ridesData);
    if (N > arrayLen) N = arrayLen; // prevenção de inputs demasiado grandes, para o modo interativo
    unsigned char driverStatus;
	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char)); // recebe free em query_requests
    result[0] = '\0';
    char * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    const partialDriverInfo * currentArrayStruct = NULL;

	int offset = 0; // acrescentar à posição atual do array, para inserir novas linhas de output
    
    for (i=1, j = N; j>0 && i<=arrayLen;i++) {
        currentArrayStruct = (*getDriverRating)(funcArgument, i); // função recebida como argumento dá partialDriverInfo struct
        driverNumber = getDriverNumber(currentArrayStruct);
        currentDriver = getDriverPtrByID(driverData,driverNumber);
        driverName = getDriverName(currentDriver);
        driverStatus = getDriverStatus(currentDriver);
        if (driverStatus == 1) {
			offset += snprintf(result + offset,STR_BUFF_SIZE,"%0*d;%s;%.3f\n", 12, driverNumber, driverName, getDriverAvgRating(currentArrayStruct));
			j--;
        }
        free(driverName);
    }
    return result;
}

//função que constroi string final no formato das Q8 e Q9, a partir de um array já ordenado
// escreve padrão diferente de string de output, em relação a topN
char * printArrayToStr(const GPtrArray * ridesArray) {
    int i, arrayLen = ridesArray->len;
	// malloc(0) pode não ser NULL
    if (arrayLen == 0) return NULL;
	char * resultStr = malloc(sizeof(char)* STR_BUFF_SIZE * arrayLen); 
	resultStr[0] = '\0';
    char * rideCity;
    Date rideDate;
    RidesStruct * currentRide = NULL;

	int offset = 0;

    for(i = 0 ; i < arrayLen; i++) {
        currentRide = (RidesStruct *) g_ptr_array_index(ridesArray,i);
        rideDate = getRideDate(currentRide);
        rideCity = getRideCity(currentRide);
		offset += snprintf(resultStr + offset, STR_BUFF_SIZE, "%0*d;%0*d/%0*d/%u;%d;%s;%.3f\n", 12, getRideID(currentRide), 2, GET_DATE_DAY(rideDate), 2, GET_DATE_MONTH(rideDate), GET_DATE_YEAR(rideDate), getRideDistance(currentRide), rideCity, getRideTip(currentRide));

		free(rideCity);
    }
    
    return resultStr;
}
