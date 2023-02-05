#ifndef STATISTICS
#define STATISTICS

#include "driverdata.h"
#include "userdata.h"
#include <stdio.h>
#include <stdlib.h>
#include "commonParsing.h"

#define REFERENCE_DAY 9
#define REFERENCE_MONTH 10
#define REFERENCE_YEAR 2022

#define REFERENCE_DATE (uint32_t)((uint32_t)REFERENCE_YEAR << 16 | (uint8_t)REFERENCE_MONTH << 8 | (uint8_t)REFERENCE_DAY)

typedef struct genderInfo genderInfo;

GArray * new_gender_array ();

void add_gender_info (GArray * maleArray, GArray * femaleArray, const DriverStruct * driver, const UserStruct * user, int rideID, int driverID); //Função que separa as rides em dois arrays e que adicona informações aos respetivos arrays 

gint sort_Q8 (gconstpointer a, gconstpointer b); //Função que da sort dos arrays começando por comparar as datas de criação de contas dos drivers, usa a mesma comparação mas com os users para desempatar e por fim se se mantiver empatado desempata pelo id da viagem

void sort_gender_array (GArray * genderArray);

char * print_array_Q8 (GArray * array, int anos); //Função que da print dos arrays para a Q8

#endif
