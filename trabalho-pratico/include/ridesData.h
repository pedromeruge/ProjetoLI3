#ifndef RIDESDATA_H
#define RIDESDATA_H

#include "commonParsing.h"
#include "driverdata.h"
#include "userdata.h"

// informações de uma ride
typedef struct RidesStruct RidesStruct;

// estrutura de dados com informações de todas as rides, divido em cidades e ordenada com base em data
typedef struct CityRides CityRides;

// estrutura de dados com informações de todas as rides, divido em riders e ordenada com base em posição no ficheiro de input
typedef struct ridesByDriverCity ridesByDriverCity;
typedef struct ridesByDriverGlobal ridesByDriverGlobal;

// informação resumida de todas as rides sobre um driver
typedef struct driverRatingInfoGlobal driverRatingInfoGlobal;
typedef struct driverRatingInfoCity driverRatingInfoCity;

// estrtura de dados com informações de todas as rides, ordenada com base em posição no ficheiro de input
typedef struct RidesData RidesData;

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
void searchCityRidesByDate(const CityRides *rides, Date dateA, Date dateB, int *res_start, int *res_end); // binary search de cityrides por data

//funções de debug
void dumpCityRides (char *, GHashTable *, CityRides *, GPtrArray *);
void dumpDriverInfoArray (char *, GPtrArray *, char *, int dataStatus);

// CityIterator * cityIteratorInit(RidesData *); // retorna um iterador (precisa de free depois)
// gboolean cityIteratorNext(CityIterator *); // itera pelas cidades
// CityRides * cityIteratorGetRides(CityIterator *); // devolve o CityRides* associado à cidade atual

// funções de rides associadas a drivers
const ridesByDriverGlobal *getRidesByDriverGlobal(const RidesData *);
const ridesByDriverGlobal *getRidesByDriverGlobalSorted(const RidesData *);
const ridesByDriverCity *getRidesByDriverCity(const CityRides *);
const driverRatingInfoGlobal *getDriverInfoGlobal(const ridesByDriverGlobal *, guint); // devolve a informação de um driver global
const driverRatingInfoCity *getDriverInfoCity(const ridesByDriverCity *, guint); // devolve a informação de um driver para uma cidade
int getRidesByDriverGlobalArraySize(const ridesByDriverGlobal *); // mudar para unsigned int ??
int getRidesByDriverCityArraySize(const ridesByDriverCity *); // mudar para unsigned int ??

double getDriverGlobalAvgRating(const driverRatingInfoGlobal *);
double getDriverCityAvgRating(const driverRatingInfoCity *);
double getDriverTipsTotal(const driverRatingInfoGlobal *);
Date getDriverMostRecRideDate(const driverRatingInfoGlobal *);
short int getDriverRidesNumber(const driverRatingInfoGlobal *);
int getDriverNumberGlobal(const driverRatingInfoGlobal *);
int getDriverNumberCity(const driverRatingInfoCity *);
short int getDriverDistTraveled(const driverRatingInfoGlobal *);

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

// RidesStruct * getRideByUser(DATA data, char *name);  // not implemented

#endif
