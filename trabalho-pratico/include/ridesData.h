#ifndef RIDESDATA_H
#define RIDESDATA_H

#include "commonParsing.h"

#define RIDES_ARR_SIZE 1000
#define SIZE 1000

typedef struct CityRides CityRides;

typedef struct RidesStruct RidesStruct;

typedef struct RidesData RidesData; // devia ser encoberto?? 

DATA getRidesData(FILE *ptr); // retorna a estrutura de dados de rides 
void freeRidesData(DATA data);  // limpa a estrutura das rides

// RidesStruct * getRideByUser(DATA data, char *name);  // not implemented
// RidesStruct * getRideByDriver(DATA data, int driver); // not implemented
CityRides * getRidesByCity(RidesData *data, char *city); // devolve todas as rides associadas a uma cidade
guint getNumberOfCityRides(CityRides *rides); // devolve o número de rides para uma cidade
RidesStruct * getCityRidesByIndex(CityRides *rides, guint ID); // devolve uma ride de uma cidade

//função para aceder ao array das rides
// funções para aceder a informações individuais das rides
RidesStruct * getRidePtrByID(RidesData * data, guint ID); // devolve a struct com toda a informação de uma ride
char * getRideDate(RidesStruct *); 
short int getRideDriver(RidesStruct *);
char * getRideUser(RidesStruct *);
char * getRideCity(RidesStruct *);
short int getRideDistance(RidesStruct *);
short int getRideScore_u(RidesStruct *);
short int getRideScore_d(RidesStruct *);
float getRideTip(RidesStruct *);
char * getRideComment(RidesStruct *);



#endif
