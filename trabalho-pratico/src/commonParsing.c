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

unsigned char getPayMethod(FILE *ptr)
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
	while (fgetc(ptr) != ';')
		;
	return result;
}

unsigned char getAccountStatus(FILE *ptr)
{
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
	return result;
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
