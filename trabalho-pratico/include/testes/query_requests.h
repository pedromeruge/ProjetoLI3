#ifndef QUERY_REQUESTS_H
#define QUERY_REQUESTS_H
#include "query_dispatch_table.h"
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
#define NS_PER_SECOND 1000000000

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td);
int fileRequests (FILE * fp, UserData *userData, DriverData *driverData, RidesData *ridesData, FILE*);

#endif
