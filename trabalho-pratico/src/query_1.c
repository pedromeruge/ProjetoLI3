#include <stdio.h>
#include <stdlib.h>
#include "query_func.h"
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"

#define STR_BUFF_SIZE 16

// void user_age(char *idstr, UserData *userData)
// {
//     int month1[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//     int present_date = 9, present_month = 10, present_year = 2022;
//     Userstruct *userAge = (Userstruct *)g_hash_table_lookup(userData, idstr);
//     char *day = strtok(userAge->birthdate, "/");
//     char *month = strtok(NULL, "/");
//     char *year = strtok(NULL, "/");
//     if (day > present_date)
//     {
//         present_date = present_date + month1[atoi(month) - 1];
//         present_month = present_month - 1;
//     }
//     if (month > present_month)
//     {
//         present_year = present_year - 1;
//         present_month = present_month + 12;
//     }
//     int final_year = present_year - atoi(year);
//     printf(final_year);
// }

// void driver_age(char *idstr, DriverStruct *driverData[])
// {
//     int month1[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//     int present_date = 9, present_month = 10, present_year = 2022;
//     DriverStruct *driverInf = getDriverPtrByID(driverData, (short int)idstr);
//     char *day = strtok(getDriverBirthdate, "/");
//     char *month = strtok(NULL, "/");
//     char *year = strtok(NULL, "/");
//     if (day > present_date)
//     {
//         present_date = present_date + month1[atoi(month) - 1];
//         present_month = present_month - 1;
//     }
//     if (month > present_month)
//     {
//         present_year = present_year - 1;
//         present_month = present_month + 12;
//     }
//     int final_year = present_year - atoi(year);
//     printf(final_year);
// }

char *query_1(char *idstr, char *trash1, char *trash2, UserData *userData, DriverStruct *driverData[], RidesData *ridesData)
{
    if (g_hash_table_contains(userData->table, idstr))
    {
        Userstruct *userInf = (Userstruct *)g_hash_table_lookup(userData->table, idstr);
        char *userResult = malloc(STR_BUFF_SIZE * sizeof(char));
        snprintf(userResult, STR_BUFF_SIZE, "%s,%c", userInf->name, userInf->gender);
        return userResult;
    }
    else
    {
        DriverStruct *driverInf = getDriverPtrByID(driverData, (short int)idstr[0]);
        char *d_name = getDriverName(driverInf);
        char *driverResult = malloc(STR_BUFF_SIZE * sizeof(char));
        snprintf(driverResult, STR_BUFF_SIZE, "%s,%c", d_name, getDriverGender(driverInf));
        free(d_name);
        return driverResult;
    }
    return 0;
}
