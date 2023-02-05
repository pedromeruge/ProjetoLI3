#include "statistics.h"

#define STR_BUFF_SIZE 64

struct genderInfo {
    const UserStruct * user;
    const DriverStruct * driver;
    int driverID;
    int id;
    Date userDate;
    Date driverDate;
};

GArray * new_gender_array () {
    return (g_array_new(FALSE, FALSE, sizeof(genderInfo)));
}

void add_gender_info (GArray * maleArray, GArray * femaleArray, const DriverStruct * driver, const UserStruct * user, int rideID, int driverID) {
	unsigned char genderDriver, genderUser;
    genderDriver = getDriverGender(driver);
	genderUser = getUserGender(user);
        if(genderDriver == genderUser && ((getUserStatus(user) == ACTIVE && getDriverStatus(driver)) == ACTIVE)) {
        genderInfo currentStruct = {user, driver, driverID, rideID, getUserAccCreation(user), getDriverAccCreation(driver)};
        if(genderDriver == M) {
		    g_array_append_vals(maleArray, &currentStruct, 1);
		} else g_array_append_vals(femaleArray, &currentStruct, 1);
	}
}

gint sort_Q8 (gconstpointer a, gconstpointer b) {
    genderInfo * gender1 = (genderInfo *) a;
    genderInfo * gender2 = (genderInfo *) b;
    gint ret = compDates(gender1->driverDate, gender2->driverDate);
    if(ret == 0) {
        ret = compDates(gender1->userDate, gender2->userDate);
        if(ret == 0) {
            ret = gender1->id - gender2->id;
        }
    }
    return ret;
}

void sort_gender_array (GArray * genderArray) {
    g_array_sort(genderArray, sort_Q8);
}

char * print_array_Q8 (GArray * array, int anos) {
    uint32_t year = REFERENCE_YEAR - anos;
    uint8_t month = REFERENCE_MONTH, day = REFERENCE_DAY;
    Date target_date = (Date) (year << 16 | month << 8 | day);
    int i, len;
    genderInfo * genderInf;
    len = array->len;
    if (len == 0) {
        return NULL;
    }
    char * resultStr = malloc(sizeof(char)* STR_BUFF_SIZE * len);
    int offset = 0;
    for(i=0; i < len; i++) {
        genderInf = &g_array_index(array, genderInfo, i);
        if (compDates(genderInf->driverDate, target_date) > 0) break;
        if (compDates(genderInf->userDate, target_date) <= 0) {
            offset += snprintf(resultStr + offset, STR_BUFF_SIZE, "%0*d;%s;%s;%s\n", 12, genderInf->driverID, getDriverName(genderInf->driver), getUserUsername(genderInf->user), getUserName(genderInf->user));
        }
    }
    if (offset == 0){
        free(resultStr);
        return NULL;
    }
    return resultStr;
}
