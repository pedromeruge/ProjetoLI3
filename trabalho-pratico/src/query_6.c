#include "query_6.h"

#define STR_BUFF_SIZE 16

char *query_6(char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
	char * city = inputStr[0], * dateA = inputStr[1], * dateB = inputStr[2];
	
	unsigned int distance = 0;
	CityRides *rides = getRidesByCity(ridesData, city);

	RidesStruct *currentRide;

	int start, end, total = 0, i;
	searchCityRidesByDate(rides, dateA, dateB, &start, &end);
	if ((start | end) < 0) return NULL;

	for (i = start; i <= end; i++) {
		currentRide = getCityRidesByIndex(rides, i);
		// A < B retorna -1
		distance += getRideDistance(currentRide);
		total ++;
	}

	if (total == 0) { free(rides); return NULL; }

	double avgDistance = (double)distance / (double)total;

	char *resultTruncated = malloc(STR_BUFF_SIZE * sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", avgDistance);
	free(rides);
	return resultTruncated;
}
