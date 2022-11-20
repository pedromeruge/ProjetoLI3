#ifndef QUERY_FUNC_H
#define QUERY_FUNC_H
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
#include "query_1.h"
#include "query_2.h"
#include "query_3.h"
#include "query_4.h"
//#include "query5.c"
#include "query_6.h"
//#include "query7.c"
//#include "query8.c"
//#include "query9.c"




// isto devia estar definido nos .h de cada querry
char * query_5 (char *, char *, char *, UserData *userData, DriverData *, RidesData *ridesData);
char * query_7 (char *, char *, char *, UserData *userData, DriverData *, RidesData *ridesData);
char * query_8 (char *, char *, char *, UserData *userData, DriverData *, RidesData *ridesData);
char * query_9 (char *, char *, char *, UserData *userData, DriverData *, RidesData *ridesData);

typedef char * query_func (char * inp1, char * inp2, char * inp3, UserData *userData, DriverData *, RidesData *ridesData);

#endif
