#include "query_1.h"

char getAge(Date date)
{
	int day = GET_DATE_DAY(date), month = GET_DATE_MONTH(date), year = GET_DATE_YEAR(date);

	int age_year = REFERENCE_YEAR - year;

	if (REFERENCE_MONTH < month || (REFERENCE_MONTH == month && REFERENCE_DAY < day)) {
		age_year--;
	}
	return age_year;
}

double driver_total_earned(int idstr, RidesData *ridesdata, DriverData *driverData)
{
    double total_spent = 0;
    double tip = 0;
    unsigned int distance[3] = {0, 0, 0}, numRides[3] = {0, 0, 0};
    DriverStruct *currentDriver = getDriverPtrByID(driverData, idstr);
    unsigned char carClass = getDriverCar(currentDriver);
    const fullDriverInfo *driver_r_Inf = getDriverGlobalInfoByID(ridesdata, idstr);
    tip = getDriverTipsTotal(driver_r_Inf);
    distance[carClass] = getDriverDistTraveled(driver_r_Inf);
    numRides[carClass] = getDriverRidesNumber(driver_r_Inf);
    total_spent = ((double)(numRides[0] * 3.25 + numRides[1] * 4 + numRides[2] * 5.2 + distance[0] * 0.62 + distance[1] * 0.79 + distance[2] * 0.94) + tip);
    return total_spent;
}

char *query_1(char * inputStr[], UserData *userData, DriverData *driverData, RidesData *ridesData) {
	char * idstr = inputStr[0];
    int numero;
    if ((numero = atoi(idstr)) != 0)
    {
		if (testDriverBounds(driverData, numero) == 1) return NULL;
        DriverStruct *driverInf = getDriverPtrByID(driverData, numero);
		if (driverInf == NULL) return NULL;
        const fullDriverInfo *driver_r_Inf = getDriverGlobalInfoByID(ridesData, numero);
        if (getDriverStatus(driverInf) == INACTIVE)
        {
            return NULL;
        }
        else
        {
            char *d_name = getDriverName(driverInf);
            Date birthdate = getDriverBirthdate(driverInf);
            char *driverResult = malloc(STR_BUFF_SIZE * sizeof(char));
            snprintf(driverResult, STR_BUFF_SIZE, "%s;%c;%d;%.3f;%d;%.3f\n", d_name, getDriverGender(driverInf), getAge(birthdate), 
            getDriverGlobalAvgRating(driver_r_Inf), getDriverRidesNumber(driver_r_Inf), driver_total_earned(numero, ridesData, driverData));
            free(d_name);
            return driverResult;
        }
    }
    else
    {
        UserStruct *userInf = getUserPtrByUsername(userData, idstr);
        if (userInf == NULL) return NULL;
		if (getUserStatus(userInf) == INACTIVE)
        {
            return NULL;
        }
        else
        {
            char *u_name = getUserName(userInf);
            Date birthdate = getUserBirthdate(userInf);
            char *userResult = malloc(STR_BUFF_SIZE * sizeof(char));
            snprintf(userResult, STR_BUFF_SIZE, "%s;%c;%d;%.3f;%d;%.3f\n", u_name, getUserGender(userInf), getAge(birthdate), 
            getAvgUserRating(userInf), getUserNumberOfRides(userInf), 
            getUserTotalSpent(userInf));
            free(u_name);
            //free(&birthdate);
            return userResult;
        }
    }
}
