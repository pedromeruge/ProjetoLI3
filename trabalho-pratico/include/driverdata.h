#ifndef DRIVERDATA_H
#define DRIVERDATA_H

#include "commonParsing.h"

#define DRIVER_ARR_SIZE 10

typedef struct {
	// id está subentendido pela posição
	char *name;
	char *birthdate;
	unsigned char gender;
	// estes podem todos ser mudados para alguma coisa melhor
	unsigned char carClass; // é usado o primeiro char da palavra, não é transformado num numero de 0 até 2
	char *licensePlate;
	char *city;
	char *accountCreation;
	unsigned char status;
} DriverStruct;

DriverStruct * getDrivers(FILE *ptr);
DATA getDriverData(FILE *ptr);
void freeDriverData(DATA data);
unsigned char getDriverCarByID(DATA data, int ID);

//apagar!!!
DriverStruct * getDriverPtrByID(DATA data, int ID);

DriverStruct * getDriverByID(DATA data, int ID);

#endif
