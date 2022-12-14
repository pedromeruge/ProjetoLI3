#ifndef CPARSING_H
#define CPARSING_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>

typedef enum {
	F = 70,
	M = 77
} Gender;

typedef enum {
	INACTIVE = 0,
	ACTIVE = 1
} AccountStatus;

typedef enum {
	CASH = 0,
	CREDIT = 1
} payMethod;

typedef enum {
	BASIC = 0,
	GREEN = 1,
	PREMIUM = 2
} CarClasses;

typedef void* DATA;

char *loadString(FILE *ptr);
void writeString(FILE *ptr, char *buffer);
int compDates (char * dateA, char * dateB);

int getPayMethod(FILE *ptr, unsigned char *res);
int getAccountStatus(FILE *ptr, unsigned char *res);
int getDate(FILE *ptr, char **res);
int getDriver(FILE *ptr, short int *res);
int getName(FILE *ptr, char **res);
int getCity(FILE *ptr, char **res);
int getDistance(FILE *ptr, short int *res);
int getScoreUser(FILE *ptr, short int *res);
int getScoreDriver(FILE *ptr, short int *res);
int getTip(FILE *ptr, float *res);
int getGender(FILE *ptr, unsigned char *res);
int getCarClass(FILE *ptr, unsigned char *res);
int getLicensePlate(FILE *ptr, char **res);

#endif
