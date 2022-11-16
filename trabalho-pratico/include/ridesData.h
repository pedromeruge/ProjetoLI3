#ifndef RIDESDATA_H
#define RIDESDATA_H

#include "commonParsing.h"

#define RIDES_ARR_SIZE 1000
#define SIZE 1000

typedef struct CityRides CityRides;

typedef struct {
	char *date;
	short int driver;
	char *user;
	char *city;
	short int distance,
	score_u,
	score_d;
	float tip;
	char *comment;
} RidesStruct;

typedef struct {
	RidesStruct **ridesArray;
	GHashTable *cityTable;
} RidesData;

RidesStruct * getRides(FILE *ptr, GHashTable *cityTable);
DATA getRidesData(FILE *ptr);
void freeRidesData(DATA data);
RidesStruct * getRideByID(DATA data, guint ID);
RidesStruct * getRideByUser(DATA data, char *name); // not implemented
RidesStruct * getRideByDriver(DATA data, int driver); // not implemented
void freeArray(void *data);
CityRides * getRidesByCity(RidesData *data, char *city);
gint compareRidesByDate (gconstpointer a, gconstpointer b);
guint getNumberOfCityRides(CityRides *rides);
RidesStruct * getRidePtrByID(DATA data, guint ID);
RidesStruct * getCityRidesByID(CityRides *rides, guint ID);

#endif
