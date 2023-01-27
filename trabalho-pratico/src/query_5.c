#include "query_5.h"

#define STR_BUFF_SIZE 16

typedef struct {
	unsigned int *m_distance,
	*m_numRides;
	DATE m_dateA, m_dateB;
	DriverData *m_driverData;
	// int reps;
} DataStruct;

void result_func (CityRides *rides, void *otherData) {
	DataStruct * data = (DataStruct *)otherData;
	// CityRides *rides = (CityRides *)cityData;
	unsigned int *distance = data->m_distance, *numRides = data->m_numRides;
	DriverData * driverData = data->m_driverData;
	DATE * dateA = &data->m_dateA, * dateB = &data->m_dateB;

	DriverStruct * currentDriver;
	RidesStruct * currentRide;
	int ID;
	unsigned char carClass;

	int start, end, i;

	searchCityRidesByDate(rides, dateA, dateB, &start, &end);
	if ((start | end) < 0) return;
	// char ola[64];
	// snprintf(ola, 64, "idk%d.txt", data->reps);
	// data->reps += 1;
	// dumpCityRidesDate(ola, rides);

	for (i = start; i <= end; i++) {
		// nunca pode ser NULL
		currentRide = getCityRidesByIndex(rides, i);
		ID = getRideDriver(currentRide);
		currentDriver = getDriverPtrByID(driverData, ID);
		// if (currentDriver != NULL) {
			carClass = getDriverCar(currentDriver);
			// A < B retorna -1
			distance[carClass] += getRideDistance(currentRide);
			numRides[carClass] += 1;
		// }
	}
}

char * query_5 (char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
	DATE * dateA = atoDate(inputStr[0]), * dateB = atoDate(inputStr[1]);

	unsigned int distance[3] = {0, 0, 0}, // basic, green, premium
		numRides[3] = {0, 0, 0};
	DataStruct data = {
		.m_distance = distance, .m_numRides = numRides, .m_dateA = *dateA, .m_dateB = *dateB, .m_driverData = driverData//, .reps = 0
	};
	iterateOverCities(ridesData, (void *)&data, result_func);

	unsigned int total = numRides[0] + numRides[1] + numRides[2];
	if (total == 0) return NULL;

	double cost = ((double)(numRides[0] * 3.25 + numRides[1] * 4 + numRides[2] * 5.2 + distance[0] * 0.62 + distance[1] * 0.79 + distance[2] * 0.94)) / (double)(total);
	char *resultTruncated = malloc(STR_BUFF_SIZE * sizeof(char));
	snprintf(resultTruncated, STR_BUFF_SIZE, "%.3f\n", cost);
	free(dateA); free(dateB);
	return resultTruncated;
}
