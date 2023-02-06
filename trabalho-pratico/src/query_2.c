#include "query_2.h"

char * query_2 (char * inputStr[], UserData *userData, DriverData * driverData, RidesData *ridesData) {
	int num = atoi(inputStr[0]);
	if (num == 0) return NULL;

    topNfunc getDriverRating = &getDriverGlobalRatingByID;
    char * result = topN(ridesData, driverData, num, ridesData, getDriverRating);

    return result;
}
