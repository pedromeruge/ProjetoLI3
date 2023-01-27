#ifndef DRIVERDATA_H
#define DRIVERDATA_H

#include "commonParsing.h"

//informações de um driver
typedef struct DriverStruct DriverStruct;

//estrutura de dados com informações de todos os drivers
typedef struct DriverData DriverData;

//funções de criar e destruir estrutura de dados dos drivers
DriverData * getDriverData(FILE *ptr); // retorna a estrutura de dados de drivers
void freeDriverData(DriverData * data); // limpa a estrutura de drivers

// receber a informação de um driver
DriverStruct * getDriverPtrByID(DriverData *, guint);

//número de drivers no ficheiro de input
int getNumberOfDrivers (DriverData *);

// funções para aceder a informações individuais de cada driver
int testDriverBounds(DriverData *data, guint ID);
char * getDriverName(DriverStruct *); // retorna o nome de um driver
void getDriverBirthdate(DATE *, DriverStruct *); // retorna a data de nascimento de um driver
unsigned char getDriverGender(DriverStruct *); // retorna o género de um driver
unsigned char getDriverCar(DriverStruct *); // retorna o tipo de carro de um driver
char * getDriverPlate(DriverStruct *); // retorna a matrícula de um driver
char * getDriverCity(DriverStruct *); // retorna a cidade de um driver
void getDriverAccCreation(DATE *,DriverStruct *); // retorna a data de criação da conta de um driver
unsigned char getDriverStatus(DriverStruct *); // retorna o estado de um driver
int driverIsValid(DriverStruct *driver); // 1 se for valido, 0 se nao for

#endif
