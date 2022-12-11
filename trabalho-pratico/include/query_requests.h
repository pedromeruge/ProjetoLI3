#ifndef QUERY_REQUESTS_H
#define QUERY_REQUESTS_H
#include "query_func.h"
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

int queryRequests (FILE * fp, UserData *userData, DriverData * driverData, RidesData *ridesData);
int writeResults (int commandN, char * strResult);
char * NOP(char * inputStr[], UserData *userData, DriverData * driverData, RidesData *ridesData);

#endif
