#include "commonParsing.h"
#include <ctype.h>

#define IF_EOF(ptr) if(loadString(ptr) == NULL) return -1;

#define BUFF_SIZE 64

// se for vazio return NULL
char *loadString(FILE *ptr)
{
	char sBuffer[128], *str = NULL;
	int i, j, chr;
	for (i = 0; (chr = fgetc(ptr)) != '\n' && chr != ';'; i++)
	{
		sBuffer[i] = (char)chr;
	}
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
	// printf("%s '%c'\n", sBuffer, (char)chr);

	return str;
}

char *safer_loadString(FILE *ptr, int *eof) {
	char sBuffer[128], *str = NULL;
	int i, j, chr;
	for (i = 0; (chr = fgetc(ptr)) != ';' && chr != EOF; i++) // este EOF é especificamente por causa dos users
	{

		sBuffer[i] = (char)chr;
	}
	if (chr == EOF) *eof = 1;
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
	int i, chr;
	for (i = 0; (chr = fgetc(ptr)) != ';' && chr != '\n'; i++)
	{
		// if (chr == '0') i--;
		// else
		buffer[i] = chr;
	}
	
	buffer[i] = '\0';
}

int p_getPayMethod(FILE *ptr, void *res)
{
	//estava a bugar com isto, aparentemente nao é preciso de qualquer das formas
	char buff[BUFF_SIZE];
	writeString(ptr, buff);
	if (buff[0] == '\0') return 0;
	return 1;
}

int p_getUserName(FILE *ptr, void *res) {
	int eof = 0;
	*(char **)res = safer_loadString(ptr, &eof);
	if (eof == 1) return -1;
	if (*(char **)res == NULL) return 0;
	return 1;
}

// esta função só é usada como a última função dos drivers e users
// logo o loadstring que é usado vai terminar num \n
int p_getAccountStatus(FILE *ptr, void *res) {
	char str[BUFF_SIZE];
	writeString(ptr, str);
	if (str[0] == '\0') return 0;

	int i;
	for (i = 0; str[i]; i++) {
		str[i] = tolower(str[i]);
	}

	unsigned char result;
	if (strncmp("active", str, 6) == 0)
	{
		result = ACTIVE;
	}
	else if (strncmp("inactive", str, 8) == 0)
	{
		result = INACTIVE;
	} else {
		return 0;
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

int p_getDate(FILE *ptr, void *res) {
	*(char **)res = loadString(ptr);
	char *date = *(char **)res; 
	if (date == NULL) return 0;
	int chars_parsed, day, month, year;
	if (sscanf(date, "%2d/%2d/%4d%n", &day, &month, &year, &chars_parsed) != 3 ||
		date[chars_parsed] != '\0' ||
		(day < 1 || day > 31) ||
		(month < 1 || month > 12)
	) {
		return 0;
	}
	return 1;
}

int p_getDriver(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	*(int *)res = atoi(tempBuffer);
	return 1;
}

int p_getName(FILE *ptr, void *res) {
	*(char **)res = loadString(ptr);
	if (*(char **)res == NULL) return 0;
	return 1;
}

int p_getCity(FILE *ptr, void *res) {
	*(char **)res = loadString(ptr);
	if (*(char **)res == NULL) return 0;
	return 1;
}

int p_getDistance(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	*(short int *)res = (short)atoi(tempBuffer);
	return 1;
}

int p_getScoreUser(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	*(short int *)res = (short)atoi(tempBuffer);
	return 1;
}

int p_getScoreDriver(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	*(short int *)res = (short)atoi(tempBuffer);
	return 1;
}

// esta função é responsavel por acabar no \n nas rides
int p_getTip(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	*(float *)res = atof(tempBuffer);
	return 1;
}

int p_getGender(FILE *ptr, void *res) {
	*(unsigned char *)res = fgetc(ptr);
	if (*(unsigned char *)res == ';') return 0;
	fseek(ptr, 1, SEEK_CUR);
	return 1;
}

int p_getCarClass(FILE *ptr, void *res) {
	// *(unsigned char *)res = (fgetc(ptr) - 97) / 6;
	// while (fgetc(ptr) != ';');
	char str[BUFF_SIZE];
	writeString(ptr, str);
	if (str[0] == '\0') return 0;
	int i;
	for(i = 0; str[i]; i++){
		str[i] = tolower(str[i]);
	}
	//car class é 0, 1 ou 2 (basic/green/premium)
	*(unsigned char *)res = (str[0] - 97) / 6;
	if (strncmp("basic", str, 5) == 0 ||
		strncmp("green", str, 5) == 0 ||
		strncmp("premium", str, 7) == 0)
	{
			return 1;
	}
	// else
	return 0;
}

int p_getLicensePlate(FILE *ptr, void *res) {
	*(char **)res = loadString(ptr);
	if (*(char **)res == NULL) return 0;
	return 1;
}

int p_getID(FILE *ptr, void *res) {
	int eof = 0;
	char *str = safer_loadString(ptr, &eof);
	if (eof == 1) {
		free(str);
		return -1;
	}
	if (str == NULL) {
		free(str);
		return 0;
	}
	*(int *)res = atoi(str);
	free(str);
	return 1;
}

// esta função automaticamente dá skip da linha que acabou de levar parse, em caso de erro ou nao
int parse_with_format(FILE *ptr, void *data, parse_format *format) {
	int i = 0, res;
	parse_func_struct *array = format->format_array;
	parse_func_struct current;
	char *field_ptr = ((char*)data);
	// res =
	// 1: correu bem
	// 0: erro
	// -1: EOF
	do {
		current = array[i];
		res = current.func(ptr, field_ptr + current.offset);
		i++;
	} while (i < (const int) format->len && res == 1);
	
	if (res == -1) { // EOF
		return -1;
	} else if (i == format->len && res == 1) { // caso normal
		return 1;
	} else { // caso de erro
		// printf("failed at %d\n", i-1);
		// damos free ao que é preciso e metemos o primeiro campo que pode levar free a NULL
		int flag = 0;
		int j;
		void ** info; // 😭😭😭😭😭😭😭

		// skip da linha
		// se for a ultima funçao assumimos que ela ja deu skip ate ao fim da linha e nao fazemos nada
		if (i != format->len) {
			while (fgetc(ptr) != '\n');
		}

		for (j = 0; j < i; j++) {
			current = array[j];
			if (current.should_free) {
				info = (void **)(field_ptr + current.offset);
				if (*info != NULL) {
					free(*info);
					if (!flag) {
						flag = 1;
						*info = NULL;
					}
				}
			}
		}
		return 0;
	}
}
