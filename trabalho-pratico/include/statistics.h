#ifndef STATISTICS
#define STATISTICS

#include "driverdata.h"
#include "userdata.h"
#include <stdio.h>
#include <stdlib.h>
#include "commonParsing.h"

#define REFERENCE_DAY 9
#define REFERENCE_MONTH 10
#define REFERENCE_YEAR 2022

#define REFERENCE_DATE (uint32_t)((uint32_t)REFERENCE_YEAR << 16 | (uint8_t)REFERENCE_MONTH << 8 | (uint8_t)REFERENCE_DAY)

typedef struct genderInfo genderInfo;

GArray * new_gender_array ();

void add_gender_info (GArray * maleArray, GArray * femaleArray, const DriverStruct * driver, const UserStruct * user, int rideID, int driverID);

gint sort_Q8 (gconstpointer a, gconstpointer b);

void sort_gender_array (GArray * genderArray);

char * print_array_Q8 (GArray * array, int anos);

#endif
