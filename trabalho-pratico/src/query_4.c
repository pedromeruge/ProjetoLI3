#include "query_4.h"

#define STR_BUFF_SIZE 16

char *query_4(char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
	char * city = inputStr[0];

	const CityRides *rides = getRidesByCity(ridesData, city);
	if (rides == NULL) return NULL; // se a cidade não eistir

	const int *numRides = getRidesTotal(rides), *distance = getRidesDistance(rides);
	double cost = (double)(numRides[0] * 3.25 + numRides[1] * 4 + numRides[2] * 5.2 + distance[0] * 0.62 + distance[1] * 0.79 + distance[2] * 0.94) / (double)(numRides[0] + numRides[1] + numRides[2]);

	// 	Basic: Tarifa mínima = 3.25€ + 0.62€/km
	//  Green: Tarifa mínima = 4.00€ + 0.79€/km
	//  Premium: Tarifa mínima = 5.20€ + 0.94€/km

	char *resultTruncated = malloc(STR_BUFF_SIZE * sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", cost);
	return resultTruncated;
}
