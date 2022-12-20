#include "query_9.h"

typedef struct {
	char *dateStart, 
        *dateEnd;
    GPtrArray * ridesInTimeFrame;
} DataStruct;

//se esta função passasse para ridesData seria mais rapida, tirando os get !
gint sort_9 (gconstpointer a, gconstpointer b) {
    const RidesStruct * ride1 = *(RidesStruct **) a;
    const RidesStruct * ride2 = *(RidesStruct **) b;
    short int ride1dist = getRideDistance(ride1),
        ride2dist = getRideDistance(ride2);
    gint result = ride2dist - ride1dist; // ordem decrescente
    if (result == 0) {
        char * ride1date = getRideDate(ride1),
             * ride2date = getRideDate(ride2);
        result = compDates(ride1date,ride2date);
        free(ride1date); free(ride2date);

        // if (result == 0) {
        //     result = 
        // }
    }
    return result;
}

void build_func (CityRides *rides, void *otherData) {
    DataStruct * data = (DataStruct *)otherData;
    char * dateStart = data->dateStart,
         * dateEnd = data->dateEnd;
    GPtrArray * ridesInTimeFrame = data->ridesInTimeFrame;

    int startDatePos, endDatePos, i;
    RidesStruct * currentRide;

    searchCityRidesByDate(rides,dateStart,dateEnd,&startDatePos,&endDatePos);

    if ((startDatePos | endDatePos) < 0) return; // é preciso??
    for (i=startDatePos;i<=endDatePos;i++) {
        currentRide = getCityRidesByIndex(rides,i);
        if (getRideTip(currentRide) > 0) { // se o user tiver dado tip
            g_ptr_array_add(ridesInTimeFrame,currentRide); // da para usar g_bytes_new_static?
        }
    }
}

char * query_9 (char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
    char * dateA = inputStr[0], * dateB = inputStr[1];

	GPtrArray * m_ridesInTimeFrame = g_ptr_array_new_with_free_func(NULL);

    DataStruct data = {.dateStart = dateA, .dateEnd = dateB, .ridesInTimeFrame = m_ridesInTimeFrame};

    iterateOverCities(ridesData,(void *) &data, build_func);
    g_ptr_array_sort(m_ridesInTimeFrame, sort_9);

    dumpCityRides ("query9",NULL,NULL,m_ridesInTimeFrame);
    g_ptr_array_free(m_ridesInTimeFrame,TRUE);
    return NULL;
}
