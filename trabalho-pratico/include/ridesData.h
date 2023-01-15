#ifndef RIDESDATA_H
#define RIDESDATA_H

#include "commonParsing.h"
#include "driverdata.h"

#define SIZE 1000

// informações de uma ride
typedef struct RidesStruct RidesStruct;

// estrutura de dados com informações de todas as rides, divido em cidades e ordenada com base em data
typedef struct CityRides CityRides;

// estrutura de dados com informações de todas as rides, divido em riders e ordenada com base em posição no ficheiro de input
typedef struct ridesByDriver ridesByDriver;

// informação resumida de todas as rides sobre um driver
typedef struct driverRatingInfo driverRatingInfo;

// estrtura de dados com informações de todas as rides, ordenada com base em posição no ficheiro de input
typedef struct RidesData RidesData;

// funções de criar e destruir estrutura de dados de rides
RidesData *getRidesData(FILE *, int); // retorna a estrutura de dados de rides
void freeRidesData(RidesData *); // limpa a estrutura de dados das rides

// funções de rides associadas a cidades
CityRides *getRidesByCity(RidesData *, char *);       // devolve todas as rides associadas a uma cidade
guint getNumberOfCityRides(CityRides *);              // devolve o número de rides para uma cidade
RidesStruct *getCityRidesByIndex(CityRides *, guint); // devolve uma ride de uma cidade

/* aplica iterator_func a todas as CityRides (ou seja, a todas as cidades)
   iterator_func tem 1 pointer para dados (CityRides de cada cidade) e outro para onde escrever resultados ou passar dados à função
   data é passado como segundo argumento da funçao */
void iterateOverCities(RidesData *, void *data, void (*iterator_func)(CityRides *, void *));
void searchCityRidesByDate(CityRides *rides, char *dateA, char *dateB, int *res_start, int *res_end); // binary search de cityrides por data

//funções de debug
void dumpCityRides (char *, GHashTable *, CityRides *, GPtrArray *);
void dumpDriverInfoArray (char *, GPtrArray *, char *, int dataStatus);

// CityIterator * cityIteratorInit(RidesData *); // retorna um iterador (precisa de free depois)
// gboolean cityIteratorNext(CityIterator *); // itera pelas cidades
// CityRides * cityIteratorGetRides(CityIterator *); // devolve o CityRides* associado à cidade atual

// funções de rides associadas a drivers
const ridesByDriver *getRidesByDriver(const RidesData *);
const ridesByDriver *getRidesByDriverSorted(const RidesData *);
const ridesByDriver *getRidesByDriverInCity(const CityRides * );
const driverRatingInfo *getDriverInfo(const ridesByDriver *, guint); // devolve a informação de um driver para todas as rides
short int getridesByDriverArraySize(const ridesByDriver *);

double getDriverAvgRating(const driverRatingInfo *);
double getDriverTipsTotal(const driverRatingInfo *);
char *getDriverMostRecRideDate(const driverRatingInfo *);
short int getDriverRidesNumber(const driverRatingInfo *);
int getDriverNumber(const driverRatingInfo *);
short int getDriverDistTraveled(const driverRatingInfo *);

// funções para dar sort ao array
gint sort_byRatings_2 (gconstpointer a, gconstpointer b); // função que compara dois elementos de um array com base em avaliação média, data de viagem mais recente e nº do condutor ( em ordem crescente), por esta ordem de importância
gint sort_byRatings_7 (gconstpointer a, gconstpointer b); // função que compara dois elementos de um array com base em avaliação média, nº do condutor (em ordem decrescente)

//void qSortArray(const ridesByDriver *driverRatingArray, gint (*sortFunction)(gconstpointer a, gconstpointer b));

//devolve os top N elementos de um array ordenado, dadas funções do resultado a escrever para uma string
// TODO: inclusivo de outros tipos de array (do tipo ridesByDriver apenas, para já)
//static char * topN(const ridesByDriver * driverRatingArray, int N, DriverData *driverData);

// receber a informação de uma ride
RidesStruct *getRidePtrByID(RidesData *, guint);

// funções de rides associadas a ID no ficheiro
//  funções para aceder a informações individuais de cada ride
int getRideID(const RidesStruct *);
char *getRideDate(const RidesStruct *);         // retorna a data de uma ride
int getRideDriver(RidesStruct *);   // retorna o ID do driver de uma ride
char *getRideUser(RidesStruct *);         // retorna o nome do user de uma ride
char *getRideCity(const RidesStruct *);         // retorna a cidade de uma ride
short int getRideDistance(const RidesStruct *); // retorna a distância de uma ride
short int getRideScore_u(RidesStruct *);  // retorna a avaliação do user de uma ride
short int getRideScore_d(RidesStruct *);  // retorna a avaliação do driver de uma ride
float getRideTip(const RidesStruct *);          // retorna a tip de uma ride
char *getRideComment(RidesStruct *);      // retorna o comentário de uma ride
int rideIsValid(RidesStruct *ride); // 1 se for valido, 0 se nao for
int getNumberOfRides(RidesData * data);

// RidesStruct * getRideByUser(DATA data, char *name);  // not implemented

#endif
