#ifndef QUERY_COMMON_FUNCS
#define QUERY_COMMON_FUNCS

#include "ridesData.h"
#include "driverdata.h"
#include "userdata.h"
#include <stdio.h>
#include <stdlib.h>
#include "commonParsing.h"

char * topN(const driversRating * driverRatingArray, int N, const DriverData *driverData);

#endif 
