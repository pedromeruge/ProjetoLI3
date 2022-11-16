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
unsigned char getPayMethod(FILE *ptr);
unsigned char getAccountStatus(FILE *ptr);
void writeString(FILE *ptr, char *buffer);
int compDates (char * dateA, char * dateB);
#endif
