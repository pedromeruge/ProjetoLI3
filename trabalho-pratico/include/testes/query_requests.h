#ifndef QUERY_REQUESTS_H
#define QUERY_REQUESTS_H
#include "query_func.h"
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

int queryRequests (FILE * fp, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);
int writeResults (int commandN, char * strResult);
#define NS_PER_SECOND 1000000000
void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td);
//apagar!!!
DriverStruct * getDriverPtrByID(DATA data, int ID);

DriverStruct * getDriverByID(DATA data, int ID);

#endif