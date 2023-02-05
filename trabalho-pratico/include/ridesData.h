#ifndef RIDESDATA_H
#define RIDESDATA_H

#include "commonParsing.h"
#include "driverdata.h"
#include "userdata.h"
#include "statistics.h"

// estrtura de dados com informações de todas as rides
typedef struct RidesData RidesData;

// informações de uma ride
typedef struct RidesStruct RidesStruct;

// estrutura de dados com informações de todas as rides, divido em cidades e ordenada com base em data
typedef struct CityRides CityRides;

//informações resumida de cada driver, abrange todas as rides (avaliação média, número, número de rides, total viajado, tips total, data mais recente de viagem)
typedef struct fullDriverInfo fullDriverInfo;
//informações de todas a rides para cada driver, pode abranger todas as rides, ou apenas as de uma cidade (avaliação média, número)
typedef struct partialDriverInfo partialDriverInfo;

// funções de criar e destruir estrutura de dados de rides
RidesData * getRidesData(FILE *ptr, const UserData *userdata, const DriverData *driverdata, char *buffer); // retorna a estrutura de dados de rides
void freeRidesData(RidesData *); // limpa a estrutura de dados das rides

// funções de rides associadas a cidades
const CityRides *getRidesByCity(const RidesData *, const char *);       // devolve todas as rides associadas a uma cidade (ou NULL se a cidade não existir)
guint getNumberOfCityRides(const CityRides *);              // devolve o número de rides para uma cidade
const RidesStruct *getCityRidesByIndex(const CityRides *, guint); // devolve uma ride de uma cidade
const int *getRidesDistance(const CityRides *rides); // devolve distance[] de uma cidade
const int *getRidesTotal(const CityRides *rides); // devolve total[] de uma cidade

/* aplica iterator_func a todas as CityRides (ou seja, a todas as cidades)
   iterator_func tem 1 pointer para dados (CityRides de cada cidade) e outro para onde escrever resultados ou passar dados à função
   data é passado como segundo argumento da funçao */
void iterateOverCities(const RidesData *, void *data, void (*iterator_func)(const CityRides *, void *));
// binary search de cityrides por data, devolve indices 'start' e 'end' através do pointer
void searchCityRidesByDate(const CityRides *rides, Date dateA, Date dateB, int *res_start, int *res_end);

//funções de debug
void dumpCityRides (char *, GHashTable *, CityRides *, GPtrArray *);

// CityIterator * cityIteratorInit(RidesData *); // retorna um iterador (precisa de free depois)
// gboolean cityIteratorNext(CityIterator *); // itera pelas cidades
// CityRides * cityIteratorGetRides(CityIterator *); // devolve o CityRides* associado à cidade atual

// funções de rides associadas a drivers

//funções que devolvem structs
const fullDriverInfo * getDriverGlobalInfoByID(const RidesData *, unsigned int); // devolve a informação de um driver global
const partialDriverInfo * getDriverGlobalRatingByID(const void * ridesData, unsigned int);
const partialDriverInfo * getDriverCityRatingByID(const void * cityData, unsigned int);

//funções que acedem a elementos das structs
double getDriverGlobalAvgRating(const fullDriverInfo *);
double getDriverTipsTotal(const fullDriverInfo *);
Date getDriverMostRecRideDate(const fullDriverInfo *);
short int getDriverRidesNumber(const fullDriverInfo *);
int getDriverNumberGlobal(const fullDriverInfo *);
short int getDriverDistTraveled(const fullDriverInfo *);

double getDriverAvgRating(const partialDriverInfo *);
int getDriverNumber(const partialDriverInfo *);
int getDriversRatingArraySize(const RidesData *); // idêntico para global e cidade, total ou parcial info, logo usa-se ridesData

//devolve os top N elementos de um array ordenado, dadas funções do resultado a escrever para uma string
// TODO: inclusivo de outros tipos de array (do tipo ridesByDriver apenas, para já)
//static char * topN(const ridesByDriver * driverRatingArray, int N, DriverData *driverData);

// receber a informação de uma ride
RidesStruct *getRidePtrByID(const RidesData *, guint);

// funções de rides associadas a ID no ficheiro
//  funções para aceder a informações individuais de cada ride
int getRideID(const RidesStruct *);
Date getRideDate(const RidesStruct *);         // retorna a data de uma ride
int getRideDriver(const RidesStruct *);   // retorna o ID do driver de uma ride
char *getRideUser(const RidesStruct *);         // retorna o nome do user de uma ride
char *getRideCity(const RidesStruct *);         // retorna a cidade de uma ride
short int getRideDistance(const RidesStruct *); // retorna a distância de uma ride
short int getRideScore_u(const RidesStruct *);  // retorna a avaliação do user de uma ride
short int getRideScore_d(const RidesStruct *);  // retorna a avaliação do driver de uma ride
float getRideTip(const RidesStruct *);          // retorna a tip de uma ride
//char *getRideComment(RidesStruct *);      // retorna o comentário de uma ride
int rideIsValid(const RidesStruct *ride); // 1 se for valido, 0 se nao for
int getNumberOfRides(const RidesData * data);

GArray * get_femaleArray (RidesData * data); // retorna o array com users e drivers do sexo feminino 
GArray * get_maleArray (RidesData * data); // retorna o array com users e drivers do sexo masculino
// RidesStruct * getRideByUser(DATA data, char *name);  // not implemented

#endif
