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
DriverStruct * getDriverPtrByID(const DriverData *, guint);

//número de drivers no ficheiro de input
int getNumberOfDrivers(const DriverData *);

// funções para aceder a informações individuais de cada driver
int testDriverBounds(const DriverData *data, guint ID);
char * getDriverName(const DriverStruct *); // retorna o nome de um driver
Date getDriverBirthdate(const DriverStruct *); // retorna a data de nascimento de um driver
unsigned char getDriverGender(const DriverStruct *); // retorna o género de um driver
unsigned char getDriverCar(const DriverStruct *); // retorna o tipo de carro de um driver
char * getDriverPlate(const DriverStruct *); // retorna a matrícula de um driver
char * getDriverCity(const DriverStruct *); // retorna a cidade de um driver
Date getDriverAccCreation(const DriverStruct *); // retorna a data de criação da conta de um driver
unsigned char getDriverStatus(const DriverStruct *); // retorna o estado de um driver
int driverIsValid(const DriverStruct *driver); // 1 se for valido, 0 se nao for

#endif
