#include "statistics.h"

#define STR_BUFF_SIZE 128

struct genderInfo {
    const UserStruct * user;
    const DriverStruct * driver;
    int driverID;
    int id;
    Date userDate;
    Date driverDate;
};

int genderInfoBSearch(GArray *array, Date target_date);

GArray * new_gender_array () {
    return (g_array_new(FALSE, FALSE, sizeof(genderInfo)));
}

//Função que separa as rides em dois arrays e que adicona informações aos respetivos arrays 
void add_gender_info (GArray * maleArray, GArray * femaleArray, const DriverStruct * driver, const UserStruct * user, int rideID, int driverID) {
	unsigned char genderDriver, genderUser;
    genderDriver = getDriverGender(driver);
	genderUser = getUserGender(user);
        if(genderDriver == genderUser && getUserStatus(user) == ACTIVE && getDriverStatus(driver) == ACTIVE) {
        genderInfo currentStruct = {user, driver, driverID, rideID, getUserAccCreation(user), getDriverAccCreation(driver)};
        if(genderDriver == 'M') {
		    g_array_append_vals(maleArray, &currentStruct, 1);
		} else g_array_append_vals(femaleArray, &currentStruct, 1);
	}
}

//Função que da sort dos arrays começando por comparar as datas de criação de contas dos drivers, usa a mesma comparação mas com os users para desempatar e por fim se se mantiver empatado desempata pelo id da viagem
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

//Função que da print dos arrays para a Q8
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
	int index = genderInfoBSearch(array, target_date);
	// if (index == 0) return NULL; ??????
    char * resultStr = malloc(sizeof(char)* STR_BUFF_SIZE * (index - 1));
    char * driverName;
    char * userUsername;
    char * userName;
    int offset = 0;

    for (i = 0; i < index-1; i++) {
        genderInf = &g_array_index(array, genderInfo, i);
        if (compDates(genderInf->userDate, target_date) <= 0) {
            driverName = getDriverName(genderInf->driver);
            userUsername = getUserUsername(genderInf->user);
            userName = getUserName(genderInf->user);
            offset += snprintf(resultStr + offset, STR_BUFF_SIZE, "%0*d;%s;%s;%s\n", 12, genderInf->driverID, driverName, userUsername, userName);
            free(driverName);
            free(userUsername);
            free(userName);
        }
    }
    if (offset == 0){
        free(resultStr);
        return NULL;
    }
    return resultStr;
}

//Binary search para encontrar o index em que o driver tenha a data target_date
int genderInfoBSearch(GArray *array, Date target_date) {
	int lim = array->len - 1,
	base = 0,
	cmp = 0,
	index = 0, len = array->len;
	genderInfo *info;
	for (; lim != 0; lim >>= 1) {
		index = base + (lim >> 1);
		info = &g_array_index(array, genderInfo, index);
		cmp = compDates(target_date, info->driverDate);
		if (cmp == 0) {
			// return index;
			break;
		}
		if (cmp > 0) {
			base = index + 1; // + 1 ??? que coisa maquiavélica
			lim--;
		}
	}

	if (cmp == 0) { // reached target date, loop forward
		for (; cmp == 0 && index < len; index ++) {
			info = &g_array_index(array, genderInfo, index);
			cmp = compDates(target_date, info->driverDate);
		}
	} else if (cmp < 0) { // target date < found date, loop forward
		for (; cmp <= 0 && index < len; index ++) {
			info = &g_array_index(array, genderInfo, index);
			cmp = compDates(target_date, info->driverDate);
		}
	} else { // target date > found date, loop backwards
		for (; cmp >= 0 && index > 0; index ++) {
			info = &g_array_index(array, genderInfo, index);
			cmp = compDates(target_date, info->driverDate);			
		}
	}
	return index;
}
