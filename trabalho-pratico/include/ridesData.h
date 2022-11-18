#ifndef RIDESDATA_H
#define RIDESDATA_H

#include "commonParsing.h"

#define RIDES_ARR_SIZE 1000
#define SIZE 1000


//informações de uma ride
typedef struct RidesStruct RidesStruct;

//?? estrutura de dados com informações de todas as rides, ordenada com base em cidade
typedef struct CityRides CityRides;

// ?? estrtura de dados com informações de todas as rides, ordenada com base em posição no ficheiro de input
typedef struct RidesData RidesData; 

//funções de criar e destruir estrutura de dados de rides
DATA getRidesData(FILE *); // retorna a estrutura de dados de rides 
void freeRidesData(DATA);  // limpa a estrutura de dados das rides

//funções de rides associadas a cidades
CityRides * getRidesByCity(RidesData *, char *); // devolve todas as rides associadas a uma cidade
guint getNumberOfCityRides(CityRides *); // devolve o número de rides para uma cidade
RidesStruct * getCityRidesByIndex(CityRides *, guint); // devolve uma ride de uma cidade

//receber a informação de uma ride
RidesStruct * getRidePtrByID(RidesData *, guint);

// funções para aceder a informações individuais de cada ride
char * getRideDate(RidesStruct *);  // retorna a data de uma ride
short int getRideDriver(RidesStruct *); // retorna o ID do driver de uma ride
char * getRideUser(RidesStruct *); // retorna o nome do user de uma ride
char * getRideCity(RidesStruct *); // retorna a cidade de uma ride
short int getRideDistance(RidesStruct *); // retorna a distância de uma ride
short int getRideScore_u(RidesStruct *); // retorna a avaliação do user de uma ride
short int getRideScore_d(RidesStruct *); // retorna a avaliação do driver de uma ride
float getRideTip(RidesStruct *); // retorna a tip de uma ride
char * getRideComment(RidesStruct *); // retorna o comentário de uma ride

// RidesStruct * getRideByUser(DATA data, char *name);  // not implemented
// RidesStruct * getRideByDriver(DATA data, int driver); // not implemented

#endif
