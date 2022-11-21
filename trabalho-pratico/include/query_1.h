#ifndef QUERRY_1
#define QUERRY_1

#include <stdio.h>
#include <stdlib.h>
#include "query_func.h"
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

#define STR_BUFF_SIZE 64
#define REFERENCE_DATE 9
#define REFERENCE_MONTH 10
#define REFERENCE_YEAR 2022

char *query_1(char *, char *, char *, UserData *, DriverData *, RidesData *);

#endif
