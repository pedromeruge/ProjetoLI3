#include "query_5.h"

#define STR_BUFF_SIZE 16

typedef struct {
	unsigned int *distance,
	*numRides;
	char *dateA, *dateB;
	DriverData *driverData;
} DataStruct;

void result_func (void *cityData, void *otherData) {
	DataStruct * data = (DataStruct *)otherData;
	CityRides *rides = (CityRides *)cityData;
	guint i, len = getNumberOfCityRides(rides);
	unsigned int *distance = data->distance, *numRides = data->numRides;
	DriverData * driverData = data->driverData;
	char *dateA = data->dateA, *dateB = data->dateB;

	DriverStruct * currentDriver;
	RidesStruct * currentRide;
	short ID;
	unsigned char carClass;
	char *currentDate;

	for (i = 0; i < len; i++) {
		currentRide = getCityRidesByIndex(rides, i);
		ID = getRideDriver(currentRide);
		currentDriver = getDriverPtrByID(driverData, ID);
		carClass = getDriverCar(currentDriver);
		currentDate = getRideDate(currentRide);
		// A < B retorna -1
		if (compDates(currentDate, dateA) >= 0) {
			if (compDates(currentDate, dateB) > 0) {
				free(currentDate);
				break;
			} else {
				distance[carClass] += getRideDistance(currentRide);
				numRides[carClass] += 1;
			}
		}
		free(currentDate);
	}
}

char * query_5 (char *dateA, char *dateB, char *trash, UserData *userData, DriverData *driverData, RidesData *ridesData) {
	unsigned int distance[3] = {0, 0, 0}, // basic, green, premium
		numRides[3] = {0, 0, 0};
	DataStruct data = {
		.distance = distance, .numRides = numRides, dateA = dateA, dateB = dateB, driverData = driverData
	};
	iterateOverCities(ridesData, (void *)&data, result_func);

	double cost = ((double)(numRides[0] * 3.25 + numRides[1] * 4 + numRides[2] * 5.2 + distance[0] * 0.62 + distance[1] * 0.79 + distance[2] * 0.94)) / (double)(numRides[0] + numRides[1] + numRides[2]);
	char *resultTruncated = malloc(STR_BUFF_SIZE * sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", cost);
	return resultTruncated;
}
