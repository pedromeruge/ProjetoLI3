#include <stdio.h>
#include <stdlib.h>
#include "query_3.h"

char * query_3(char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
    int num = atoi(inputStr[0]);
    if (num == 0) return NULL; 
    char * result = userTopN(userData, num);
    return result;
}
