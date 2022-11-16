#ifndef QUERRY_2
#define QUERRY_2

#include "ridesData.h"
#include "driverdata.h"
#include "userdata.h"
#include "commonParsing.h"

typedef struct {
    void  * ratingChart; // void * porque vai guardar um array de ratings de 1 a 5, e mais tarde é convertido para um valor médio desses ratings
    char * mostRecRideDate;
    short int driverNumber; // talvez meter em int o valor, ocupa menos espaço com char?
} driverRatingInfo;

char * strResults(GPtrArray * driverRatingArray, int N, DriverStruct *driverData[]);
void freeRidesRating (void * DriverRidesRating);
driverRatingInfo * newDriverRating (char *, short int, short int);
char * query_2(char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);

#endif
