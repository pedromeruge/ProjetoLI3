#ifndef DRIVERDATA_H
#define DRIVERDATA_H

#include "commonParsing.h"

#define DRIVER_ARR_SIZE 10

typedef struct DriverStruct DriverStruct;

typedef struct DriverData DriverData;

DriverData * getDriverData(FILE *ptr); // fica -> main
void freeDriverData(DriverData * data); // fica -> main

DriverStruct * getDriverPtrByID(DriverData *, guint);

char * getDriverName(DriverStruct *);
char * getDriverBirthdate(DriverStruct *);
unsigned char getDriverGender(DriverStruct *);
unsigned char getDriverCar(DriverStruct *);
char * getDriverPlate(DriverStruct *);
char * getDriverCity(DriverStruct *);
char * getDriverAccCreation(DriverStruct *);
unsigned char getDriverStatus(DriverStruct *);

#endif
