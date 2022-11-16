#include "query_4.h"

#define STR_BUFF_SIZE 16

char * query_4(char *city, char *trash1, char *trash2, UserData *userData, DriverStruct *driverData[], RidesData *ridesData) {
	guint i;
	unsigned int distance[3] = {0, 0, 0}, //basic, green, premium
	numRides[3] = {0, 0, 0};
	GPtrArray *array = getRidesByCity(ridesData, city);
	RidesStruct *currentRide;
	DriverStruct *currentDriver;
	guint len = array->len;

	for (i = 0; i < len; i++) {
		currentRide = g_ptr_array_index(array, i);
		currentDriver = getDriverByID(driverData, currentRide->driver);

		// 		  -97 /6
		// b = 97  0  0
		// g = 103 6  1
		// p = 112 15 2
		distance[(currentDriver->carClass - 97)/6] += currentRide->distance;
		numRides[(currentDriver->carClass - 97)/6] += 1;

		// printf("%d %d %d\n", distance[0], distance[1], distance[2]);
	}
	// 	Basic: Tarifa mínima = 3.25€ + 0.62€/km
	//  Green: Tarifa mínima = 4.00€ + 0.79€/km
	//  Premium: Tarifa mínima = 5.20€ + 0.94€/km
	
	float cost = (numRides[0]*3.25 + numRides[1]*4 + numRides[2]*5.2\
	+ distance[0]*0.62 + distance[1]*0.79 + distance[2]*0.94) / array->len;
	
	char *resultTruncated = malloc(STR_BUFF_SIZE*sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", cost);
	return resultTruncated;
}
