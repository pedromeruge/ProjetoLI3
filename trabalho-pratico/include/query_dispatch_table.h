#ifndef QUERY_DISPATCH_TABLE_H
#define QUERY_DISPATCH_TABLE_H

#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

#include "query_1.h"
#include "query_2.h"
#include "query_3.h"
#include "query_4.h"
#include "query_5.h"
#include "query_6.h"
#include "query_7.h"
//#include "query8.h"
#include "query_9.h"

char * query_8 (char * inputStr[], UserData *userData, DriverData *, RidesData *ridesData);

typedef char * query_func (char * inputStr[], UserData *userData, DriverData *, RidesData *ridesData);

#endif
