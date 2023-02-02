#include "query_common_funcs.h"

#define STR_BUFF_SIZE 64

char * topN(const RidesData * ridesData, const DriverData * driverData, int N, const void * funcArgument, topNfunc getDriverRating) {
    int i, j, driverNumber, arrayLen = getDriversRatingArraySize(ridesData);
    if (N > arrayLen) N = arrayLen; // prevenção de inputs demasiado grandes
    unsigned char driverStatus;
	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char)); // recebe free em query_requests
    result[0] = '\0';
    char * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    const partialDriverInfo * currentArrayStruct = NULL;

	int offset = 0; // acrescentar à posição atual do array, para inserir novas linhas de output
    
    for (i=1, j = N; j>0 && i<=arrayLen;i++) {
        currentArrayStruct = (*getDriverRating)(funcArgument, i);
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

