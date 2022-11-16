#include "driverdata.h"

#define SIZE 1000
#define DRIVER_STR_BUFF 32

struct DriverStruct {
	// id está subentendido pela posição
	char *name;
	char *birthdate;
	unsigned char gender;
	// estes podem todos ser mudados para alguma coisa melhor
	unsigned char carClass; // é usado o primeiro char da palavra para converter em 0, 1 ou 2
	char *licensePlate;
	char *city;
	char *accountCreation;
	unsigned char status;
};

DriverStruct * getDrivers(FILE *ptr) {
	int i, tempchr, count, chr;
	// char *name;
	DriverStruct *driverStructArray = malloc(SIZE*sizeof(DriverStruct));

	for (i = count = 0; i < SIZE; i++, count++) {
		while ((chr = fgetc(ptr)) != ';');// && chr != -1); // skip id
		// name = loadString(ptr);
		driverStructArray[i].name = loadString(ptr);
		// if (name == NULL) break;
		driverStructArray[i].birthdate = loadString(ptr);
		driverStructArray[i].gender = fgetc(ptr); fseek(ptr, 1, SEEK_CUR);
		driverStructArray[i].carClass = (fgetc(ptr) - 97)/6; while (fgetc(ptr) != ';');
		driverStructArray[i].licensePlate = loadString(ptr);
		driverStructArray[i].city = loadString(ptr);
		driverStructArray[i].accountCreation = loadString(ptr);
		driverStructArray[i].status = getAccountStatus(ptr);

		// avaçar até proxima linha
		while ((tempchr = fgetc(ptr)) != '\n');// && (tempchr != -1));
	}

	// for (i = 0; i < count; i++) {
	// 	printf("%s %s %d %c %s %s %s %d\n", driverStructArray[i].name,
	// 	driverStructArray[i].birthdate,
	// 	driverStructArray[i].gender,
	// 	driverStructArray[i].carClass,
	// 	driverStructArray[i].licensePlate,
	// 	driverStructArray[i].city,
	// 	driverStructArray[i].accountCreation,
	// 	driverStructArray[i].status);
	// }

	return driverStructArray;
}

DATA getDriverData(FILE *ptr) {
	DriverStruct **driverData = malloc(DRIVER_ARR_SIZE*sizeof(DriverStruct *));

	int i;
	while (fgetc(ptr) != '\n'); // avançar a primeira linha (tbm podia ser um seek hardcoded)
	for (i = 0; i < DRIVER_ARR_SIZE; i++) driverData[i] = getDrivers(ptr);

	return driverData;
}

void freeDriverData(DATA data) {
	DriverStruct **driverData = data;
	int i, j;
	DriverStruct *segment, block;
	for (i = 0; i < DRIVER_ARR_SIZE; i++) {
		segment = driverData[i];
		for (j = 0; j < SIZE; j++) {
			block = segment[j];
			free(block.name);
			free(block.birthdate);
			free(block.licensePlate);
			free(block.city);
			free(block.accountCreation);
		}
		free(segment);
	}
	free(driverData);
}

//devolve a struct(dados) associada ao driver número i
DriverStruct * getDriverPtrByID(DATA data, short int ID) {
	ID -= 1;
	int i = ID / SIZE;
	DriverStruct **primaryArray = data,
	*secondaryArray = primaryArray[i],
	*result = &(secondaryArray[ID - SIZE*i]);
	return result;
}

char * getDriverName(DriverStruct * driver) {
	return strndup(driver->name,DRIVER_STR_BUFF);
}

char * getDriverBirthdate(DriverStruct * driver){
	return strndup(driver->birthdate,DRIVER_STR_BUFF);
}

unsigned char getDriverGender(DriverStruct * driver) {
	return (driver->gender);
}

unsigned char getDriverCar(DriverStruct * driver) {
	return (driver->carClass);
}
char * getDriverPlate(DriverStruct * driver) {
	return (driver->licensePlate);
}

char * getDriverCity(DriverStruct * driver) {
	return (driver->city);
}

char * getDriverAccCreation(DriverStruct * driver) {
	return (driver->accountCreation);
}

unsigned char getDriverStatus(DriverStruct * driver){
	return (driver->status);
}
