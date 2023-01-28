#ifndef QUERRY_1
#define QUERRY_1

#include <stdio.h>
#include <stdlib.h>
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

#define STR_BUFF_SIZE 64
#define REFERENCE_DAY 9
#define REFERENCE_MONTH 10
#define REFERENCE_YEAR 2022

#define REFERENCE_DATE (uint32_t)((uint32_t)REFERENCE_YEAR << 16 | (uint8_t)REFERENCE_MONTH << 8 | (uint8_t)REFERENCE_DAY)

char * query_1(char * inputStr[], UserData *, DriverData *, RidesData *);

#endif
