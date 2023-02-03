#include "statistics.h"

struct genderInfo {
    const UserStruct * user;
    const DriverStruct * driver;
    int id;
    Date userDate;
    Date driverDate;
};

GArray * new_gender_array () {
    return (g_array_new(FALSE, FALSE, sizeof(genderInfo)));
}

void add_gender_info (GArray * maleArray, GArray * femaleArray, const DriverStruct * driver, const UserStruct * user, int rideID) {
	unsigned char genderDriver, genderUser;
    genderDriver = getDriverGender(driver);
	genderUser = getUserGender(user);
    if(genderDriver == genderUser && ((getUserStatus(user) == getDriverStatus(driver)) == ACTIVE)) {
        genderInfo currentStruct = {user, driver, rideID, getUserAccCreation(user), getDriverAccCreation(driver)};
        if(genderDriver == M) {
		    g_array_append_vals(maleArray, &currentStruct, 1);
		} else g_array_append_vals(femaleArray, &currentStruct, 1);
	}
}

