#include "query_common_funcs.h"

#define STR_BUFF_SIZE 64

char * topNGlobal(const ridesByDriverGlobal * driverRatingArray, int N, const DriverData *driverData) {
    int i, j, driverNumber, arrayLen = getRidesByDriverGlobalArraySize(driverRatingArray);
    unsigned char driverStatus;
	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char)); // recebe free em query_requests
    result[0] = '\0';
    char * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    const driverRatingInfoGlobal * currentArrayStruct = NULL;

	int offset = 0;
    
    for (i=arrayLen, j = N; j>0 && i>0 ;i--) {
        currentArrayStruct = getDriverInfoGlobal(driverRatingArray, i);
        driverNumber = getDriverNumberGlobal(currentArrayStruct);
        currentDriver = getDriverPtrByID(driverData,driverNumber);
        driverName = getDriverName(currentDriver);
        driverStatus = getDriverStatus(currentDriver);
        if (driverStatus == 1) {
			offset += snprintf(result + offset,STR_BUFF_SIZE,"%0*d;%s;%.3f\n", 12, driverNumber, driverName, getDriverGlobalAvgRating(currentArrayStruct));
			j--;
        }
        free(driverName);
    }
    return result;
}

char * topNCity(const ridesByDriverCity * driverRatingArray, int N, const DriverData *driverData) {
    int i, j, driverNumber, arrayLen = getRidesByDriverCityArraySize(driverRatingArray);
    unsigned char driverStatus;
	char * result = malloc( N * STR_BUFF_SIZE * sizeof(char)); // recebe free em query_requests
    result[0] = '\0';
    char * driverName = NULL;
    DriverStruct * currentDriver = NULL;
    const driverRatingInfoCity * currentArrayStruct = NULL;

	int offset = 0;
    
    for (i=arrayLen, j = N; j>0 && i>0 ;i--) {
        currentArrayStruct = getDriverInfoCity(driverRatingArray, i);
        driverNumber = getDriverNumberCity(currentArrayStruct);
        currentDriver = getDriverPtrByID(driverData,driverNumber);
        driverName = getDriverName(currentDriver);
        driverStatus = getDriverStatus(currentDriver);
        if (driverStatus == 1) {
			offset += snprintf(result + offset,STR_BUFF_SIZE,"%0*d;%s;%.3f\n", 12, driverNumber, driverName, getDriverCityAvgRating(currentArrayStruct));
			j--;
        }
        free(driverName);
    }
    return result;
}
