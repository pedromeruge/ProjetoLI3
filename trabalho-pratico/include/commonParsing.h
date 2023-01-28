#ifndef CPARSING_H
#define CPARSING_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <stdint.h>

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
	CREDIT = 1,
	DEBIT = 2
} payMethodEnum;

typedef enum {
	BASIC = 0,
	GREEN = 1,
	PREMIUM = 2
} CarClasses;

typedef uint32_t Date;

#define GET_DATE_YEAR(date) (date >> 16)
#define GET_DATE_MONTH(date) ((date & (uint32_t)0x0000FF00) >> 8)
#define GET_DATE_DAY(date) (date & (uint32_t)0x000000FF)

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

// char *loadString(FILE *ptr);
void writeString(FILE *ptr, char *buffer);
int compDates(Date dateA, Date dateB);

int p_getPayMethod(FILE *ptr, void *res);
int p_getAccountStatus(FILE *ptr, void *res);
int p_getUserName(FILE *ptr, void *res);
int p_getDate(FILE *ptr, void *res);
int p_getDriver(FILE *ptr, void *res);
int p_getName(FILE *ptr, void *res);
int p_getCity(FILE *ptr, void *res);
int p_getDistance(FILE *ptr, void *res);
int p_getScoreUser(FILE *ptr, void *res);
int p_getScoreDriver(FILE *ptr, void *res);
int p_getTip(FILE *ptr, void *res);
int p_getGender(FILE *ptr, void *res);
int p_getCarClass(FILE *ptr, void *res);
int p_getLicensePlate(FILE *ptr, void *res);
int p_getID(FILE *ptr, void *res);
int p_getDriverID(FILE *ptr, void *res);
int p_getComment(FILE *ptr, void *res);
Date atoDate(char *str);

int parse_with_format(FILE *ptr, void *data, parse_format *format);
void dumpWithFormat(void *data, parse_format *format);

#endif
