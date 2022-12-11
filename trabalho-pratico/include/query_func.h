#ifndef QUERY_FUNC_H
#define QUERY_FUNC_H
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
#include "query_1.h"
#include "query_2.h"
#include "query_3.h"
#include "query_4.h"
#include "query_5.h"
#include "query_6.h"
//#include "query7.c"
//#include "query8.c"
//#include "query9.c"

char * query_7 (char * inputStr[], UserData *userData, DriverData *, RidesData *ridesData);
char * query_8 (char * inputStr[], UserData *userData, DriverData *, RidesData *ridesData);
char * query_9 (char * inputStr[], UserData *userData, DriverData *, RidesData *ridesData);

typedef char * query_func (char * inputStr[], UserData *userData, DriverData *, RidesData *ridesData);

#endif
