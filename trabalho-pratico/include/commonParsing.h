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

typedef int parse_func (FILE *ptr, void *res);

// esta struct tem de ser conhecida para módulos exteriores
typedef struct {
	parse_func *func;
	size_t offset;
	char should_free;
} parse_func_struct;

typedef struct {
	int len;
	parse_func_struct *format_array;
} parse_format;

char *loadString(FILE *ptr);
void writeString(FILE *ptr, char *buffer);
int compDates (char * dateA, char * dateB);

int getPayMethod(FILE *ptr, void *res);
int getAccountStatus(FILE *ptr, void *res);
int getDate(FILE *ptr, void *res);
int getDriver(FILE *ptr, void *res);
int getName(FILE *ptr, void *res);
int getCity(FILE *ptr, void *res);
int getDistance(FILE *ptr, void *res);
int getScoreUser(FILE *ptr, void *res);
int getScoreDriver(FILE *ptr, void *res);
int getTip(FILE *ptr, void *res);
int getGender(FILE *ptr, void *res);
int getCarClass(FILE *ptr, void *res);
int getLicensePlate(FILE *ptr, void *res);

int parse_with_format(FILE *ptr, void *data, parse_format *format);

#endif
