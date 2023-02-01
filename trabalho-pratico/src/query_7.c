#include "query_7.h"
#include "query_2.h" // temporário

//TODO: função getRidesbyDriverInCity guardar o array criado numa hash table para cada cidade
char * query_7(char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
    int num = atoi(inputStr[0]);
    char * city = inputStr[1];

    //podia ser mais eficiente, a função getRidesByDriverinCity chama addDriverInfo que calcula inclusive a ride mais recente, o que n é perciso para esta querry, meter uma opção 0 ou 1 na função para distinguir?
    const CityRides * cityRides = getRidesByCity(ridesData,city);
    if (cityRides == NULL) return NULL;

	const driversRating * driverRatingArray = getDriversRatingCity (cityRides);

    char * result = topN(driverRatingArray, num, driverData);

    free((driversRating *) driverRatingArray);

    return result;
}
