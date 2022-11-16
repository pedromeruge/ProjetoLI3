#include "driverdata.h"

#define SIZE 1000

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
		driverStructArray[i].carClass = fgetc(ptr); while (fgetc(ptr) != ';');
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
DriverStruct * getDriverByID(DATA data, int ID) {
	ID -= 1;
	int i = ID / SIZE;
	DriverStruct **primaryArray = data,
	*secondaryArray = primaryArray[i],
	*result = &(secondaryArray[ID - SIZE*i]);
	return result;
}

DriverStruct * getDriverPtrByID(DATA data, int ID) {
	ID -= 1;
	int i = ID / SIZE;
	DriverStruct **primaryArray = data,
	*secondaryArray = primaryArray[i],
	*result = &(secondaryArray[ID - SIZE*i]);
	return result;
}

unsigned char getDriverCarByID(DATA data, int ID) {
	return ((DriverStruct *)getDriverPtrByID(data, ID))->carClass;
}
