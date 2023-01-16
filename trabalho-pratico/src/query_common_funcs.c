#include "query_common_funcs.h"

#define STR_BUFF_SIZE 64

char * topN(const ridesByDriver * driverRatingArray, int N, DriverData *driverData) {
    int i, j, driverNumber, arrayLen = getridesByDriverArraySize(driverRatingArray);
    unsigned char driverStatus;
	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char)); // recebe free em query_requests
    if (result == NULL) exit (1);
    result[0] = '\0';
    char temp[STR_BUFF_SIZE], * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    const driverRatingInfo * currentArrayStruct = NULL;
    
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
	//printf("result %s\n", result);
    return result;
}
