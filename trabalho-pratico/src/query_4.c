#include "query_4.h"

#define STR_BUFF_SIZE 16

char *query_4(char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
	guint i;
	char * city = inputStr[0];
	
	unsigned int distance[3] = {0, 0, 0}, // basic, green, premium
		numRides[3] = {0, 0, 0};
	CityRides *rides = getRidesByCity(ridesData, city);
	if (rides == NULL) return NULL; // se a cidade não eistir

	RidesStruct *currentRide;
	DriverStruct *currentDriver;
	unsigned char carClass;

	guint len = getNumberOfCityRides(rides);
	if (len == 0) return NULL;
	int ID;
	for (i = 0; i < len; i++)
	{
		// nunca pode ser NULL
		currentRide = getCityRidesByIndex(rides, i);
		ID = getRideDriver(currentRide);
		currentDriver = getDriverPtrByID(driverData, ID);
		if (currentDriver != NULL) {
			carClass = getDriverCar(currentDriver);
			distance[carClass] += getRideDistance(currentRide);
			numRides[carClass] += 1;
		}
		// printf("%d %d %d\n", distance[0], distance[1], distance[2]);
	}
	// 	Basic: Tarifa mínima = 3.25€ + 0.62€/km
	//  Green: Tarifa mínima = 4.00€ + 0.79€/km
	//  Premium: Tarifa mínima = 5.20€ + 0.94€/km

	double cost = ((double)(numRides[0] * 3.25 + numRides[1] * 4 + numRides[2] * 5.2 + distance[0] * 0.62 + distance[1] * 0.79 + distance[2] * 0.94)) / (double)len;

	char *resultTruncated = malloc(STR_BUFF_SIZE * sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", cost);
	return resultTruncated;
}
