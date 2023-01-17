#ifndef QUERY_REQUESTS_H
#define QUERY_REQUESTS_H

#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

#include "query_dispatch_table.h"

int interactRequests(UserData *userData, DriverData *driverData, RidesData *ridesData); // modo interativo
int batchRequests (FILE * fp, UserData *userData, DriverData * driverData, RidesData *ridesData); // modo batch

#endif
