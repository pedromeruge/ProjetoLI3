#include "query_9.h"
#include "commonParsing.h"
#include <stdio.h>
#include <stdlib.h>

#define STR_BUFF_SIZE 50

// typedef struct {
//     const RidesStruct * currentRide;
//     int rideID;
// } RidesStructWithID;

typedef struct {
	Date dateStart, 
         dateEnd;
    GPtrArray * ridesInTimeFrame;
} DataStruct;

//meter no query_common_funcs a receber tipos diferentes: da Q9 e da Q8
// não pode ser igual a topN porque essa função escreve padrão diferente na string, e lê do fim para o início
//função que constroi a string final, a partir do array já ordenado com base nos parâmetros fornecidos
char * printArrayToStr(const GPtrArray * ridesArray) {
    int i, arrayLen = ridesArray->len;
    char * resultStr = malloc(sizeof(char)* STR_BUFF_SIZE * arrayLen); // malloc muito grande, talvez particionar em array de strings com BUFFER_SIZE (1000 talvez?)
    // if (resultStr == NULL) return NULL; // if malloc fails
    resultStr[0] = '\0';
    char * rideCity;
    Date rideDate;
    RidesStruct * currentRide = NULL;

	int offset = 0;

    for(i = 0 ; i < arrayLen; i++) {
        currentRide = (RidesStruct *) g_ptr_array_index(ridesArray,i);
        rideDate = getRideDate(currentRide);
        rideCity = getRideCity(currentRide);
		offset += snprintf(resultStr + offset, STR_BUFF_SIZE, "%0*d;%0*d/%0*d/%u;%d;%s;%.3f\n", 12, getRideID(currentRide), 2, GET_DATE_DAY(rideDate), 2, GET_DATE_MONTH(rideDate), GET_DATE_YEAR(rideDate), getRideDistance(currentRide), rideCity, getRideTip(currentRide));

		//free(rideDate);
		free(rideCity);
    }
    
    return resultStr;
}

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
//função usada em iterateOverCities; 
void build_func (const CityRides *rides, void *otherData) {
    DataStruct * data = (DataStruct *)otherData;
    Date dateStart = data->dateStart,
         dateEnd = data->dateEnd;
    GPtrArray * ridesInTimeFrame = data->ridesInTimeFrame;

    int startDatePos, endDatePos, i;
    const RidesStruct * currentRide;

    searchCityRidesByDate(rides, dateStart, dateEnd, &startDatePos, &endDatePos);

    if ((startDatePos | endDatePos) < 0) return; // é preciso??
    for (i = startDatePos; i <= endDatePos; i++) { // mal!!! // tem de se incluir int nas ridesStruct
        currentRide = getCityRidesByIndex(rides, i);
        if (getRideTip(currentRide) > 0) { // se o user tiver dado tip
            g_ptr_array_add(ridesInTimeFrame, (gpointer)currentRide); // da para usar g_bytes_new_static?
        }
    }
}

char * query_9 (char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
    Date dateA = atoDate(inputStr[0]), dateB = atoDate(inputStr[1]);

	GPtrArray * m_ridesInTimeFrame = g_ptr_array_new_with_free_func(NULL); // mudar para new full
    DataStruct data = {.dateStart = dateA, .dateEnd = dateB, .ridesInTimeFrame = m_ridesInTimeFrame};

    iterateOverCities(ridesData,(void *) &data, build_func);
    g_ptr_array_sort(m_ridesInTimeFrame, sort_9);

    char * result = printArrayToStr(m_ridesInTimeFrame);

    g_ptr_array_free(m_ridesInTimeFrame,TRUE);
    return result;
}
