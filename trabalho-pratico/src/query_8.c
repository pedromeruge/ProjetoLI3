#include <stdio.h>
#include <stdlib.h>
#include "query_8.h"

char * query_8 (char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
    char gender = (inputStr[0])[0];
    if(gender == 'M'){
        return print_array_Q8(get_maleArray(ridesData), atoi(inputStr[1]));
    } else return print_array_Q8(get_femaleArray(ridesData), atoi(inputStr[1]));
}
