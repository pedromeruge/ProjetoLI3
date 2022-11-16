#ifndef QUERY_REQUESTS_H
#define QUERY_REQUESTS_H
#include "query_func.h"
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

int queryRequests (FILE * fp, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);
int writeResults (int commandN, char * strResult);

#endif
