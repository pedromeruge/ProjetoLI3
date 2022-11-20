#include "query_6.h"

#define STR_BUFF_SIZE 16

char *query_6(char *city, char *dateA, char *dateB, UserData *userData, DriverData *driverData, RidesData *ridesData) {
	guint i, total;
	unsigned int distance = 0;
	CityRides *rides = getRidesByCity(ridesData, city);

	RidesStruct *currentRide;

	char *currentDate;

	guint len = getNumberOfCityRides(rides);

	for (i = total = 0; i < len; i++)
	{
		currentRide = getCityRidesByIndex(rides, i);
		currentDate = getRideDate(currentRide);
		// A < B retorna -1
		if (compDates(currentDate, dateA) >= 0) {
			if (compDates(currentDate, dateB) > 0) {
				free(currentDate);
				break;
			} else {
				distance += getRideDistance(currentRide);
				total++;
			}
		}
		free(currentDate);
	
		// printf("%d %d %d\n", distance[0], distance[1], distance[2]);
	}

	if (total == 0) { free(rides); return NULL; }

	double avgDistance = (double)distance / (double)total;

	char *resultTruncated = malloc(STR_BUFF_SIZE * sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", avgDistance);
	free(rides);
	return resultTruncated;
}
