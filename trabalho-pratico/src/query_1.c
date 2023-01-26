#include "query_1.h"

char getAge(char* date)
{
    int month1[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int present_date = REFERENCE_DATE;
    int present_month = REFERENCE_MONTH;
    int present_year = REFERENCE_YEAR;
    char *day = strtok(date, "/");
    char *month = strtok(NULL, "/");
    char *year = strtok(NULL, "/");
    if (atoi(day) > present_date)
    {
        present_date = present_date + month1[atoi(month) - 1];
        present_month = present_month - 1;
    }
    if (atoi(month) > present_month)
    {
        present_year = present_year - 1;
        present_month = present_month + 12;
    }
    int final_year = present_year - atoi(year);
    return final_year;
}

double driver_total_earned(int idstr, RidesData *ridesdata, DriverData *driverData)
{
    double total_spent = 0;
    double tip = 0;
    unsigned int distance[3] = {0, 0, 0}, numRides[3] = {0, 0, 0};
    DriverStruct *currentDriver = getDriverPtrByID(driverData, idstr);
    unsigned char carClass = getDriverCar(currentDriver);
    const ridesByDriver *ridesInf = getRidesByDriver(ridesdata);
    const driverRatingInfo *driver_r_Inf = getDriverInfo(ridesInf, idstr);
    tip = getDriverTipsTotal(driver_r_Inf);
    distance[carClass] = getDriverDistTraveled(driver_r_Inf);
    numRides[carClass] = getDriverRidesNumber(driver_r_Inf);
    total_spent = ((double)(numRides[0] * 3.25 + numRides[1] * 4 + numRides[2] * 5.2 + distance[0] * 0.62 + distance[1] * 0.79 + distance[2] * 0.94) + tip);
    free((ridesByDriver *)ridesInf);
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
        const ridesByDriver *ridesInf = getRidesByDriver(ridesData);
        const driverRatingInfo *driver_r_Inf = getDriverInfo(ridesInf, numero);
        if (getDriverStatus(driverInf) == INACTIVE)
        {
            free((ridesByDriver *)ridesInf);
            return NULL;
        }
        else
        {
            char *d_name = getDriverName(driverInf);
            char *birthdate = getDriverBirthdate(driverInf);
            char *driverResult = malloc(STR_BUFF_SIZE * sizeof(char));
            snprintf(driverResult, STR_BUFF_SIZE, "%s;%c;%d;%.3f;%d;%.3f\n", d_name, getDriverGender(driverInf), getAge(birthdate), 
            getDriverAvgRating(driver_r_Inf), getDriverRidesNumber(driver_r_Inf), driver_total_earned(numero, ridesData, driverData));
            free(d_name);
            free(birthdate);
            free((ridesByDriver *)ridesInf);
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
            char *birthdate = getUserBirthdate(userInf);
            char *userResult = malloc(STR_BUFF_SIZE * sizeof(char));
            snprintf(userResult, STR_BUFF_SIZE, "%s;%c;%d;%.3f;%d;%.3f\n", u_name, getUserGender(userInf), getAge(birthdate), 
            getAvgUserRating(userInf), getUserNumberOfRides(userInf), 
            getUserTotalSpent(userInf));
            free(u_name);
            free(birthdate);
            return userResult;
        }
    }
}
