#ifndef RIDESDATA_H
#define RIDESDATA_H

#include "commonParsing.h"
#include "driverdata.h"
#include "userdata.h"
#include "statistics.h"

// ########
// ### estrutura principal
// ########

// contém todas as estruturas com informações e estatísticas das rides
typedef struct RidesData RidesData;

// funções de criar e destruir estrutura de dados de rides
RidesData * getRidesData(FILE *ptr, const UserData *userdata, const DriverData *driverdata, char *buffer); // retorna a estrutura de dados de rides
void freeRidesData(RidesData *); // limpa a estrutura de dados das rides

// ########
// ### estrutura com as informações de rides que aparecem no ficheiro de input
// ########

// informações de uma ride, idênticas às lidas do ficheiro 
typedef struct RidesStruct RidesStruct;

//funções que acedem a informações de cada ride, lidas do ficheiro
int getRideID(const RidesStruct *);
Date getRideDate(const RidesStruct *);// retorna a data de uma ride
int getRideDriver(const RidesStruct *);// retorna o ID do driver de uma ride
char *getRideUser(const RidesStruct *);// retorna o nome do user de uma ride
char *getRideCity(const RidesStruct *);// retorna a cidade de uma ride
short int getRideDistance(const RidesStruct *);// retorna a distância de uma ride
short int getRideScore_u(const RidesStruct *);// retorna a avaliação do user de uma ride
short int getRideScore_d(const RidesStruct *);// retorna a avaliação do driver de uma ride
float getRideTip(const RidesStruct *);// retorna a tip de uma ride
//char *getRideComment(RidesStruct *);// retorna o comentário de uma ride

//RidesStruct *getRidePtrByID(const RidesData *, guint); // receber as informações de uma ride
//int getNumberOfRides(const RidesData * data); // número total de rides


// ########
// ### estrutura com informações de todas as rides, divida em cidades
// ########

// estruturas associadas a uma cidade
typedef struct CityRides CityRides;

// funções que devolvem estruturas e informações de uma cidade
const CityRides *getRidesByCity(const RidesData *, const char *); // devolve todas as rides associadas a uma cidade (ou NULL se a cidade não existir)
guint getNumberOfCityRides(const CityRides *); // devolve o número de rides para uma cidade
const RidesStruct *getCityRidesByIndex(const CityRides *, guint); // devolve uma ride de uma cidade
const int *getRidesDistance(const CityRides *rides); // devolve distance[] de uma cidade
const int *getRidesTotal(const CityRides *rides); // devolve total[] de uma cidade

/* aplica iterator_func a todas as CityRides (ou seja, a todas as cidades)
   iterator_func tem 1 pointer para dados (CityRides de cada cidade) e outro para onde escrever resultados ou passar dados à função
   data é passado como segundo argumento da funçao */
void iterateOverCities(const RidesData *, void *data, void (*iterator_func)(const CityRides *, void *));
// binary search de cityrides por data, devolve indices 'start' e 'end' através do pointer
void searchCityRidesByDate(const CityRides *rides, Date dateA, Date dateB, int *res_start, int *res_end);


// ########
// #### funções de rides associadas a estatísticas de cada driver
// ########

//informações totais resumidas de cada driver (avaliação média, número, número de rides, total viajado, tips total, data mais recente de viagem), abrange todas as rides 
typedef struct fullDriverInfo fullDriverInfo;
//informações parciais resumidas de cada driver (avaliação média, número), pode abranger o total das rides, ou apenas as de uma cidade 
typedef struct partialDriverInfo partialDriverInfo;

//funções que devolvem structs
const fullDriverInfo * getDriverGlobalInfoByID(const RidesData *, unsigned int); //obter a info global de um driver, a partir de um ID
const partialDriverInfo * getDriverGlobalRatingByID(const void * ridesData, unsigned int); //obter a info parcial de um driver, a partir de um ID, ordenado por parâmetros da Q2
const partialDriverInfo * getDriverCityRatingByID(const void * cityData, unsigned int); //obter a info parcial de um driver, a partir de um ID, ordenado por parâmetros da Q7

//funções que acedem a informações das structs
//infomrações totais
double getDriverGlobalAvgRating(const fullDriverInfo *);
double getDriverTipsTotal(const fullDriverInfo *);
Date getDriverMostRecRideDate(const fullDriverInfo *);
short int getDriverRidesNumber(const fullDriverInfo *);
int getDriverNumberGlobal(const fullDriverInfo *);
short int getDriverDistTraveled(const fullDriverInfo *);
//informações parciais
double getDriverAvgRating(const partialDriverInfo *);
int getDriverNumber(const partialDriverInfo *);
int getDriversRatingArraySize(const RidesData *); // idêntico para global e cidade, total ou parcial info, logo usa-se ridesData

// função que verifica se uma ride é válida
int rideIsValid(const RidesStruct *ride); // 1 se for valido, 0 se nao for

GArray * get_femaleArray (RidesData * data); // retorna o array com users e drivers do sexo feminino 
GArray * get_maleArray (RidesData * data); // retorna o array com users e drivers do sexo masculino
// RidesStruct * getRideByUser(DATA data, char *name);  // not implemented

#endif
