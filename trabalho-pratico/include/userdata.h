#ifndef USERDATA_H
#define USERDATA_H

#include "driverdata.h"
#include "commonParsing.h"

//informações de um user
typedef struct UserStruct UserStruct;

//estrutura de dados com informações de todos os users
typedef struct UserData UserData;

//funções para criar, manipular e destruir a estrutura de dados de users
UserData * getUserData(FILE *ptr, char *buffer); // retorna a estrutura de dados de users
void freeUserData(UserData * userdata); // limpa a estrutura de dados de users 
void add_user_info (const UserData* data, const DriverData* driverdata, const char* name, int driver, int distance, int score, float tip,  Date date); // adiciona informação de um user à estrutura de dados de users
void sort_userarray (const UserData * data); // ordena a estrutura de dados de users com base nos parâmetros da Q3

//devolve uma string com informações sobre os top N users
char * userTopN (const UserData * data, int N);

//receber a informação de um user(ou NULL se a cidade não existir)
UserStruct * getUserPtrByUsername(const UserData *, const char *);

//funções para aceder a informações de cada user
char * getUserName (const UserStruct * data); // retorna o nome de um user (diferente do nome de input em GetUserPtrByName)
unsigned char getUserGender(const UserStruct *); // retorna o género de um user
Date getUserBirthdate(const UserStruct* data); // retorna a data de nascimento de um user
Date getUserAccCreation (const UserStruct * data); // retorna a data de criação de conta de um user
unsigned char getUserPayMethod(const UserStruct * data); // retorna o método de pagamento de um user
unsigned char getUserStatus (const UserStruct * data); // retorna o estado de um user
int userIsValid(const UserStruct *user); // 1 se for valido, 0 se nao for
double getUserTotalSpent (const UserStruct* user); // retorna o total que um user gastou
double getAvgUserRating (const UserStruct* user); // retorna a avaliação média de um user
int getUserNumberOfRides (const UserStruct* user); //retorna o número de viagens de um user
char *getUserUsername(const UserStruct *data); // retorna o username de um user
//getUserName -> tem de já ser sabido, para poder aceder a outras informações do user

#endif
