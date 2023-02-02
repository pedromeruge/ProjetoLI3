#include "query_common_funcs.h"

#define STR_BUFF_SIZE 64

char * topN(const RidesData * ridesData, const DriverData * driverData, int N, const void * funcArgument, const partialDriverInfo * (*getDriverRating)(const void *, unsigned int)) {
    int i, j, driverNumber, arrayLen = getDriversRatingArraySize(ridesData);
    unsigned char driverStatus;
	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char)); // recebe free em query_requests
    result[0] = '\0';
    char * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    const partialDriverInfo * currentArrayStruct = NULL;

	int offset = 0;
    
    for (i=arrayLen, j = N; j>0 && i>0 ;i--) {
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

