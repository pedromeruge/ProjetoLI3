#include "query_1.h"

char user_age(char *idstr, UserData *userData)
{
    int month1[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int present_date = REFERENCE_DATE;
    int present_month = REFERENCE_MONTH;
    int present_year = REFERENCE_YEAR;
    UserStruct *userAge = getUserPtrByUsername(userData, idstr);
    char *u_age = getUserBirthdate(userAge);
    char *day = strtok(u_age, "/");
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
    free(u_age);
    return final_year;
}

char driver_age(char *idstr, DriverData *driverData)
{
    int month1[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int present_date = REFERENCE_DATE;
    int present_month = REFERENCE_MONTH;
    int present_year = REFERENCE_YEAR;
    DriverStruct *driverAge = getDriverPtrByID(driverData, atoi(idstr));
    char *d_age = getDriverBirthdate(driverAge);
    char *day = strtok(d_age, "/");
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
    free(d_age);
    return final_year;
}

int user_sum_trips(char *idstr, UserData *userData, RidesData *ridesdata)
{
    int ridesArr = RIDES_ARR_SIZE * SIZE, i, sum = 0;
    for (i = 0; i < ridesArr; i++)
    {
        RidesStruct *currentRide = getRidePtrByID(ridesdata, i + 1);
        char *ride_user = getRideUser(currentRide);
        if (strcmp(ride_user, idstr) == 0)
        {
            sum += 1;
        }
        free(ride_user);
    }
    return sum;
}

double user_avg_ev(char *idstr, UserData *UserData, RidesData *ridesdata)
{
    int ridesArr = RIDES_ARR_SIZE * SIZE, i;
    double sum = 0;
    for (i = 0; i < ridesArr; i++)
    {
        RidesStruct *currentRide = getRidePtrByID(ridesdata, i + 1);
        char *ride_user = getRideUser(currentRide);
        if (strcmp(ride_user, idstr) == 0)
        {
            sum += getRideScore_u(currentRide);
        }
        free(ride_user);
    }
    return sum / user_sum_trips(idstr, UserData, ridesdata);
}

double user_total_spent(char *idstr, UserData *UserData, RidesData *ridesdata, DriverData *driverData)
{
    int ridesArr = RIDES_ARR_SIZE * SIZE, i;
    double total_spent = 0;
    unsigned int distance[3] = {0, 0, 0}, numRides[3] = {0, 0, 0};
    double tip = 0;
    for (i = 0; i < ridesArr; i++)
    {
        RidesStruct *currentRide = getRidePtrByID(ridesdata, i + 1);
        int driver_ID = getRideDriver(currentRide);
        DriverStruct *currentDriver = getDriverPtrByID(driverData, driver_ID);
        char *ride_user = getRideUser(currentRide);
        unsigned char carClass = getDriverCar(currentDriver);
        if (strcmp(ride_user, idstr) == 0)
        {
            distance[carClass] += getRideDistance(currentRide);
            numRides[carClass] += 1;
            tip += (double)getRideTip(currentRide);
        }
        free(ride_user);
    }
    total_spent = ((double)(numRides[0] * 3.25 + numRides[1] * 4 + numRides[2] * 5.2 + distance[0] * 0.62 + distance[1] * 0.79 + distance[2] * 0.94) + tip);
    return total_spent;
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

char *query_1(char *idstr, char *trash1, char *trash2, UserData *userData, DriverData *driverData, RidesData *ridesData)
{
    int numero;
    if ((numero = atoi(idstr)) != 0)
    {
        DriverStruct *driverInf = getDriverPtrByID(driverData, numero);
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
            char *driverResult = malloc(STR_BUFF_SIZE * sizeof(char));
            snprintf(driverResult, STR_BUFF_SIZE, "%s;%c;%d;%.3f;%d;%.3f\n", d_name, getDriverGender(driverInf), driver_age(idstr, driverData), getDriverAvgRating(driver_r_Inf), getDriverRidesNumber(driver_r_Inf), driver_total_earned(numero, ridesData, driverData));
            free(d_name);
            free((ridesByDriver *)ridesInf);
            return driverResult;
        }
    }
    else
    {
        UserStruct *userInf = getUserPtrByUsername(userData, idstr);
        if (getUserStatus(userInf) == INACTIVE)
        {
            return NULL;
        }
        else
        {
            char *u_name = getUserName(userInf);
            char *userResult = malloc(STR_BUFF_SIZE * sizeof(char));
            snprintf(userResult, STR_BUFF_SIZE, "%s;%c;%d;%.3f;%d;%.3f\n", u_name, getUserGender(userInf), user_age(idstr, userData), user_avg_ev(idstr, userData, ridesData), user_sum_trips(idstr, userData, ridesData), user_total_spent(idstr, userData, ridesData, driverData));
            free(u_name);
            return userResult;
        }
    }
}
