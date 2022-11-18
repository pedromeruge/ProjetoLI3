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
	// 	Basic: Tarifa mínima = 3.25€ + 0.62€/km
	//  Green: Tarifa mínima = 4.00€ + 0.79€/km
	//  Premium: Tarifa mínima = 5.20€ + 0.94€/km

	float avgDistance = (float)distance / (float)total;

	char *resultTruncated = malloc(STR_BUFF_SIZE * sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", avgDistance);
	free(rides);
	return resultTruncated;
}
