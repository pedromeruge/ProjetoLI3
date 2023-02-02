#include "query_9.h"
#include "commonParsing.h"
#include <stdio.h>
#include <stdlib.h>

#define STR_BUFF_SIZE 50

//struct passada à build_func para usar no iterateOverCities
typedef struct {
	Date dateStart, 
         dateEnd;
    GPtrArray * ridesInTimeFrame;
} DataStruct;

//se esta função passasse para ridesData seria mais rapida, tirando os get !
// função que faz o sort do array de rides, conforme os parâmetros fornecidos
gint sort_9 (gconstpointer a, gconstpointer b) {
    RidesStruct * ride1 = *(RidesStruct **) a;
    RidesStruct * ride2 = *(RidesStruct **) b;
    short int ride1dist = getRideDistance(ride1),
    ride2dist = getRideDistance(ride2);
    gint result = (gint) (ride2dist - ride1dist); // ordem decrescente

    if (result == 0) {
        Date ride1date = getRideDate(ride1),
        ride2date = getRideDate(ride2);
        result = compDates(ride2date, ride1date); // ordem decrescente
        //free(ride1date); 
        //free(ride2date);

        if (result == 0) {
            int ride1ID = getRideID(ride1),
            ride2ID = getRideID(ride2);
            result = (int) (ride2ID - ride1ID); // ordem decrescente
        }
    }
    return result;
}

// função que acrescenta a um array novo todas as rides que entrem no intervalo de tempo fornecido, em que o user tenha dado gorjeta
//função usada em iterateOverCities
void build_func (const CityRides *rides, void *otherData) {
    DataStruct * data = (DataStruct *)otherData;
    Date dateStart = data->dateStart,
         dateEnd = data->dateEnd;
    GPtrArray * ridesInTimeFrame = data->ridesInTimeFrame;

    int startDatePos, endDatePos, i;
    const RidesStruct * currentRide;

    searchCityRidesByDate(rides, dateStart, dateEnd, &startDatePos, &endDatePos);

    if ((startDatePos | endDatePos) < 0) return;
    for (i = startDatePos; i <= endDatePos; i++) {
        currentRide = getCityRidesByIndex(rides, i);
        if (getRideTip(currentRide) > 0) { // se o user tiver dado tip
            g_ptr_array_add(ridesInTimeFrame, (gpointer)currentRide);
        }
    }
}

char * query_9 (char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
    Date dateA = atoDate(inputStr[0]), dateB = atoDate(inputStr[1]);

	GPtrArray * myRidesInTimeFrame = g_ptr_array_new_full(1000,NULL); // função de free é NULL, porque são apenas pointers para rides, levam free noutro sítio
    DataStruct data = {.dateStart = dateA, .dateEnd = dateB, .ridesInTimeFrame = myRidesInTimeFrame};

    iterateOverCities(ridesData,(void *) &data, build_func);
    g_ptr_array_sort(myRidesInTimeFrame, sort_9);

    char * result = printArrayToStr(myRidesInTimeFrame);

    g_ptr_array_free(myRidesInTimeFrame,TRUE);
    return result;
}
