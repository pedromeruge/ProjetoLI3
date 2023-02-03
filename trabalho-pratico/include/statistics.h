#ifndef STATISTICS
#define STATISTICS

#include "driverdata.h"
#include "userdata.h"
#include <stdio.h>
#include <stdlib.h>
#include "commonParsing.h"

typedef struct genderInfo genderInfo;

GArray * new_gender_array ();

void add_gender_info (GArray * maleArray, GArray * femaleArray, const DriverStruct * driver, const UserStruct * user, int rideID);

// void print_func (const GArray * maleArray);

#endif
