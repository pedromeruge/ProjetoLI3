#include "query_7.h"
#include "query_2.h" // temporário

char * query_7(char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
    int num = atoi(inputStr[0]);
    char * city = inputStr[1];

     const CityRides * cityRides = getRidesByCity(ridesData,city);
    if (cityRides == NULL) return NULL;

    topNfunc getDriverRating = &getDriverCityRatingByID;
    char * result = topN(ridesData, driverData, num, cityRides, getDriverRating);

    return result;
}
