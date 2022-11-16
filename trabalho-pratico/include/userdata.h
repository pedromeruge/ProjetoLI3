#ifndef USERDATA_H
#define USERDATA_H

#include "commonParsing.h"

#define USER_ARR_SIZE 100

typedef struct {
	// char *username;
	char *name;
	unsigned char gender;
	char *birthdate;
	char *accountCreation;
	unsigned char payMethod;
	unsigned char status;
} Userstruct;

typedef struct {
	GHashTable *table;
} UserData;

DATA getUserData(FILE *ptr);
void freeTableData(void *userData);
void freeUserData(DATA userdata);

#endif
