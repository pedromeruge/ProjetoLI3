#include "commonParsing.h"

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

	// putchar('\n');

	// sBuffer[i] = '\0';
	// printf("buffer:%s | string:%s\n", sBuffer, str);

	return str;
}

// esta tem poucas condiçoes, pode dar erro
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

int getPayMethod(FILE *ptr, unsigned char *res)
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
	*res = result;
	return 1;
}

int getAccountStatus(FILE *ptr, unsigned char *res) {
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
	*res = result;
	return 1;
}

// compara se date1 é menor que date2
/*
int compDates (char * date1, char * date2) {
	int day1,month1,year1,day2,month2,year2;
	sscanf(date1, "%d/%d/%d", &day1,&month1,&year1);
	sscanf(date2, "%d/%d/%d", &day2,&month2,&year2);
	int result = ((year1 - year2) * 64) + ((month1-month2) * 16) + ((day1-day2)); // peso do ano arbitrariamente maior e do mês segundo maior
	return result;
}
*/

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

int getDate(FILE *ptr, char **res) {
	*res = loadString(ptr);
	return 1;
}

int getDriver(FILE *ptr, short int *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*res = (short)atoi(tempBuffer);
	return 1;
}

int getName(FILE *ptr, char **res) {
	*res = loadString(ptr);
	return 1;
}

int getCity(FILE *ptr, char **res) {
	*res = loadString(ptr);
	return 1;
}

int getDistance(FILE *ptr, short int *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*res = (short)atoi(tempBuffer);
	return 1;
}

int getScoreUser(FILE *ptr, short int *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*res = (short)atoi(tempBuffer);
	return 1;
}

int getScoreDriver(FILE *ptr, short int *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*res = (short)atoi(tempBuffer);
	return 1;
}

int getTip(FILE *ptr, float *res) {
	char tempBuffer[16];
	writeString(ptr, tempBuffer);
	*res = atof(tempBuffer);
	return 1;
}

int getGender(FILE *ptr, unsigned char *res) {
	*res = fgetc(ptr);
	fseek(ptr, 1, SEEK_CUR);
	return 1;
}

int getCarClass(FILE *ptr, unsigned char *res) {
	*res = (fgetc(ptr) - 97) / 6;
	while (fgetc(ptr) != ';');
	return 1;
}

int getLicensePlate(FILE *ptr, char **res) {
	*res = loadString(ptr);
	return 1;
}
