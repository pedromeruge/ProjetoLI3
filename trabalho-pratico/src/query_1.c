#include <stdio.h>
#include <stdlib.h>
#include "query_func.h"
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
#include "query_1.h"

#define STR_BUFF_SIZE 64

char user_age(char *idstr, UserData *userData)
{
    int month1[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int present_date = 9, present_month = 10, present_year = 2022;
    UserStruct *userAge = getUserPtrByUsername(userData, idstr);
    char *day = strtok(getUserBirthdate(userAge), "/");
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
    printf("%d", final_year);
    return final_year;
}

char driver_age(char *idstr, DriverData *driverData)
{
    int month1[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int present_date = 9, present_month = 10, present_year = 2022;
    DriverStruct *driverAge = getDriverPtrByID(driverData, atoi(idstr));
    char *day = strtok(getDriverBirthdate(driverAge), "/");
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
    printf("%d", final_year);
    return final_year;
}

char *query_1(char *idstr, char *trash1, char *trash2, UserData *userData, DriverData *driverData, RidesData *ridesData)
{
    int numero;
    if ((numero = atoi(idstr)) != 0)
    {
        DriverStruct *driverInf = getDriverPtrByID(driverData, numero);
        if (getDriverStatus(driverInf) == INACTIVE)
        {
            return NULL;
        }
        char *d_name = getDriverName(driverInf);
        char *driverResult = malloc(STR_BUFF_SIZE * sizeof(char));
        snprintf(driverResult, STR_BUFF_SIZE, "%s;%c;%d\n", d_name, getDriverGender(driverInf), driver_age(idstr, driverData));
        free(d_name);
        return driverResult;
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
            char *userResult = malloc(STR_BUFF_SIZE * sizeof(char));
            snprintf(userResult, STR_BUFF_SIZE, "%s;%c;%d\n", getUserName(userInf), getUserGender(userInf), user_age(idstr, userData));
            return userResult;
        }
    }
}
