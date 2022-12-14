#include "commonParsing.h"

// se for vazio return NULL
char *loadString(FILE *ptr)
{
	char sBuffer[128], *str = NULL;
	int i, j, chr;
	for (i = 0; (chr = fgetc(ptr)) != '\n' && chr != ';'; i++)
	{ // && (chr != -1); i++) {
		// putchar(chr);

		sBuffer[i] = (char)chr;
	}
	// é preciso o if?????
	if (i > 0)
	{
		str = malloc(sizeof(char) * (i + 1));
		for (j = 0; j < i; j++)
		{
			str[j] = sBuffer[j];
		}
		str[j] = '\0';
	}

	// sBuffer[i] = '\0';
	// printf("buffer:%s | string:%s\n", sBuffer, str);

	return str;
}

// se for vazio buffer começa com \0
void writeString(FILE *ptr, char *buffer)
{
	// buffer is 16 bytes
	int i, chr;
	for (i = 0; (chr = fgetc(ptr)) != ';'; i++)
	{
		// if (chr == '0') i--;
		// else
		buffer[i] = chr;
	}
	
	buffer[i] = '\0';
}

int getPayMethod(FILE *ptr, void *res)
{
	fseek(ptr, 1, SEEK_CUR); // avançar o 'c' para comparar 'a' vs 'r' (c|ash vs c|redit)
	char chr = fgetc(ptr);
	unsigned char result;
	if (chr == 'a')
	{
		result = CASH;
	}
	else
	{
		result = CREDIT;
	}
	while (fgetc(ptr) != ';');
	*(unsigned char *)res = result;
	return 1;
}

int getAccountStatus(FILE *ptr, void *res) {
	char chr = fgetc(ptr);
	unsigned char result;
	if (chr == 'a')
	{
		result = ACTIVE;
	}
	else
	{
		result = INACTIVE;
	}
	*(unsigned char *)res = result;
	return 1;
}

int compDates(char *dateA, char *dateB)
{
	// DD/MM/YYYY
	int res;
	if ((res = strncmp(dateA + 6, dateB + 6, 4)) != 0)
	{
		return res;
	}
	else if ((res = strncmp(dateA + 3, dateB + 3, 2)) != 0)
	{
		return res;
	} else {
		return strncmp(dateA, dateB, 2);
	}
}

int getDate(FILE *ptr, void *res) {
	*(char **)res = loadString(ptr);
	return 1;
}

int getDriver(FILE *ptr, void *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*(short int *)res = (short)atoi(tempBuffer);
	return 1;
}

int getName(FILE *ptr, void *res) {
	*(char **)res = loadString(ptr);
	if (res == NULL) return 0;
	return 1;
}

int getCity(FILE *ptr, void *res) {
	*(char **)res = loadString(ptr);
	if (res == NULL) return 0;
	return 1;
}

int getDistance(FILE *ptr, void *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*(short int *)res = (short)atoi(tempBuffer);
	return 1;
}

int getScoreUser(FILE *ptr, void *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*(short int *)res = (short)atoi(tempBuffer);
	return 1;
}

int getScoreDriver(FILE *ptr, void *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*(short int *)res = (short)atoi(tempBuffer);
	return 1;
}

int getTip(FILE *ptr, void *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*(float *)res = atof(tempBuffer);
	return 1;
}

int getGender(FILE *ptr, void *res) {
	*(unsigned char *)res = fgetc(ptr);
	fseek(ptr, 1, SEEK_CUR);
	return 1;
}

int getCarClass(FILE *ptr, void *res) {
	*(unsigned char *)res = (fgetc(ptr) - 97) / 6;
	while (fgetc(ptr) != ';');
	return 1;
}

int getLicensePlate(FILE *ptr, void *res) {
	*(char **)res = loadString(ptr);
	if (res == NULL) return 0;
	return 1;
}

#include "driverdata.h"

int parse_with_format(FILE *ptr, void *data, parse_format *format) {
	int i = 0, res;
	parse_func_struct *array = format->format_array;
	parse_func_struct current;
	char *field_ptr = ((char*)data);
	do {
		current = array[i];
		res = current.func(ptr, field_ptr + current.offset);
		i++;
	} while (i < (const int) format->len && res);

	if (i == format->len) {
		return 1;
	} else {
		// damos free ao que é preciso e metemos o primeiro campo que pode levar free a NULL
		int flag = 0;
		for (i = 0; i < (const int) format->len; i++) {
			current = array[i];
			if (current.should_free) {
				free(field_ptr + current.offset);
				if (!flag) {
					flag = 1;
					*(void **)(field_ptr + current.offset) = NULL;
				}
			}
		}
		return 0;
	}
}
