#ifndef RIDESDATA_H
#define RIDESDATA_H

#include "commonParsing.h"

#define RIDES_ARR_SIZE 1000
#define SIZE 1000

typedef struct CityRides CityRides;

typedef struct RidesStruct RidesStruct;

typedef struct {
	RidesStruct **ridesArray;
	GHashTable *cityTable;
} RidesData;

DATA getRidesData(FILE *ptr); // fica -> main
void freeRidesData(DATA data);  // fica -> main
// RidesStruct * getRideByUser(DATA data, char *name);  // not implemented
// RidesStruct * getRideByDriver(DATA data, int driver); // not implemented
CityRides * getRidesByCity(RidesData *data, char *city); // fica -> querries
guint getNumberOfCityRides(CityRides *rides); // fica -> queeries
RidesStruct * getRidePtrByID(DATA data, guint ID); // fica -> querries
RidesStruct * getCityRidesByID(CityRides *rides, guint ID); // fica -> querries (incompleta, deve devolver copia nova)

#endif
