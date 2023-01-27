#ifndef USERDATA_H
#define USERDATA_H

#include "driverdata.h"
#include "commonParsing.h"

//informações de um user
typedef struct UserStruct UserStruct;

//estrutura de dados com informações de todos os users
typedef struct UserData UserData;

//funções para criar e destruir estrutura de dados de users
UserData * getUserData(FILE *ptr); // retorna a estrutura de dados de rides 
void freeUserData(UserData * userdata); // limpa a estrutura de dados das rides

//receber a informação de um user
UserStruct * getUserPtrByUsername(UserData *, char *);

void add_user_info (UserData* data, DriverData* driverdata, char* name, int driver, int distance, int score, float tip, DATE * date);

void sort_userarray (UserData * data);

gint userDistComp (gconstpointer a, gconstpointer b);

char * userTopN (UserData * data, int N);

//funções para aceder a informações de cada user
            //getUserName -> tem de já ser sabido, para poder aceder a outras informações do user
char * getUserName (UserStruct * data); // retorna o nome de um user (diferente do nome de input em GetUserPtrByName)
unsigned char getUserGender(UserStruct *); // retorna o género de um user
void getUserBirthdate(DATE *, UserStruct* data); // retorna a data de nascimento de um user
void getUserAccCreation (DATE *, UserStruct * data); // retorna a data de criação de conta de um user
unsigned char getUserPayMethod(UserStruct * data); // retorna o método de pagamento de um user
unsigned char getUserStatus (UserStruct * data); // retorna o estado de um user
int userIsValid(UserStruct *user); // 1 se for valido, 0 se nao for
double getUserTotalSpent (UserStruct* user);
double getAvgUserRating (UserStruct* user);
int getUserNumberOfRides (UserStruct* user);

#endif
