#include "query_6.h"

#define STR_BUFF_SIZE 16

char *query_6(char *city, char *dateA, char *dateB, UserData *userData, DriverData *driverData, RidesData *ridesData) {
	unsigned int distance = 0;
	CityRides *rides = getRidesByCity(ridesData, city);

	RidesStruct *currentRide;

	int start = searchCityRidesByDate(rides, dateA, 0), end = searchCityRidesByDate(rides, dateB, 1), i;
	if (end == -1) end = getNumberOfCityRides(rides) - 1;
	
	int total = 0;
	if (start != -1) {
		total = 0;
		for (i = start; i <= end; i++) {
			currentRide = getCityRidesByIndex(rides, i);
			// A < B retorna -1
			distance += getRideDistance(currentRide);
			total ++;
		}
	}

	if (total == 0) { free(rides); return NULL; }

	double avgDistance = (double)distance / (double)total;

	char *resultTruncated = malloc(STR_BUFF_SIZE * sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", avgDistance);
	free(rides);
	return resultTruncated;
}
