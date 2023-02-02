#ifndef QUERY_COMMON_FUNCS
#define QUERY_COMMON_FUNCS

#include "ridesData.h"
#include "driverdata.h"
#include "userdata.h"
#include <stdio.h>
#include <stdlib.h>
#include "commonParsing.h"

typedef const partialDriverInfo * (*topNfunc)(const void * arg1, unsigned int);

char * topN(const RidesData *, const DriverData *, int N, const void * funcArg1, topNfunc);

#endif 
