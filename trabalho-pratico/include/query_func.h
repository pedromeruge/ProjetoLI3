#ifndef QUERY_FUNC_H
#define QUERY_FUNC_H
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
//#include "query2.c"
//#include "query3.c"
#include "query_4.h"
//#include "query5.c"
//#include "query6.c"
//#include "query7.c"
//#include "query8.c"
//#include "query9.c"




// isto devia tar definido nos .h de cada querry
char * query_1 (char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);
char * query_2 (char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);
char * query_3 (char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);

char * query_5 (char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);
char * query_6 (char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);
char * query_7 (char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);
char * query_8 (char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);
char * query_9 (char *, char *, char *, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);

typedef char * query_func (char * inp1, char * inp2, char * inp3, UserData *userData, DriverStruct *driverData[], RidesData *ridesData);

#endif
