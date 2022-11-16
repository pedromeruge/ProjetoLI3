#include <stdio.h>
#include <stdlib.h>
#include "query_func.h"
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

char * query_1 (char * idstr, char * trash1, char * trash2, UserData *userData, DriverStruct *driverData[], RidesData *ridesData) {
    // int id = atoi(idstr);
    // Userstruct * loadUser = userData[0];
    char * b = malloc(sizeof(char) * 5);
    b = strcpy(b,"oula");
    return (b);
}
