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
	unsigned char carClass; // é usado o primeiro char da palavra para converter em 0, 1 ou 2
	char *licensePlate;
	char *city;
	char *accountCreation;
	unsigned char status;
} DriverStruct;

DATA getDriverData(FILE *ptr); // fica -> main
void freeDriverData(DATA data); // fica -> main

DriverStruct * getDriverPtrByID(DATA data, short int ID);
unsigned char getDriverCar(DriverStruct *);

#endif
