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

// datas são guardadas como um int: 16 bits para ano, 8 bits para mês, 8 bits para o dia
typedef uint32_t Date;

#define GET_DATE_YEAR(date) (date >> 16)
#define GET_DATE_MONTH(date) ((date & (uint32_t)0x0000FF00) >> 8)
#define GET_DATE_DAY(date) (date & (uint32_t)0x000000FF)

// returns:
// 0 se houver erro
// > 0 se nao
// funções de parse usadas no formato da parse_with_format
typedef int parse_func (char *buffer, int *bp, void *res);

#define SIZE_OF_READ_BUFFER (1 << 16)

// esta struct tem de ser conhecida para módulos exteriores
typedef struct {
	parse_func *func;
	size_t offset;
	char should_free;
} parse_func_struct;

// a função de parse faz o parse da struct num loop, consoante o array de formatos
typedef struct {
	// numero de elementos no array
	int len;
	// array de formatos
	parse_func_struct *format_array;
} parse_format;

// char *loadString(FILE *ptr);
void writeString(FILE *ptr, char *buffer);
// compara duas datas
int compDates(Date dateA, Date dateB);

// funções de get de diferentes campos
int p_getString(char *buffer, int *bp, void *res);
int p_getGender(char *buffer, int *bp, void *res);
int p_getID(char *buffer, int *bp, void *res);
int p_getDate(char *buffer, int *bp, void *res);
int p_getPayMethod(char *buffer, int *bp, void *res);
int p_getAccountStatus(char *buffer, int *bp, void *res);
int p_checkEmpty(char *buffer, int *bp, void *res);
int p_getCarClass(char *buffer, int *bp, void *res);
int p_getShortPositiveInt(char *buffer, int *bp, void *res);
int p_getTip(char *buffer, int *bp, void *res);
int p_checkEmptyNewline(char *buffer, int *bp, void *res);

// string to Date
Date atoDate(const char *str);

// faz o parse de uma struct numm loop seguindo o formato
int parse_with_format(FILE *ptr, void *data, const parse_format *format, int *bp, int *sp, char *buffer);
// debug
void dumpWithFormat(void *data, parse_format *format);

#endif
