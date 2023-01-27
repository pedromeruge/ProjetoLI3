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

// este nao faz check de newline!!!
int safer_writeString(FILE *ptr, char *buffer)
{
	int i, chr;
	for (i = 0; (chr = fgetc(ptr)) != ';' && chr != EOF; i++)
	{
		// if (chr == '0') i--;
		// else
		buffer[i] = chr;
	}
	
	buffer[i] = '\0';
	if (chr == EOF) return -1;
	if (i == 0) return 0;
	return 1;
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

int compDates(DATE * dateA, DATE * dateB) {
	// DD/MM/YYYY
	int res;
	if ((res = (dateA->year - dateB->year)) != 0)
		return res;
	else if ((res = dateA->month - dateB->month) != 0)
		return res;
	else
		return ((dateA->day) - (dateB->day));
}

//TODO: mudar para structa = structb, é mais eficiente que memcpy!!
inline void dateDup (DATE * dest, const DATE * src) {
	*dest = *src;
}

DATE * atoDate (char * date) {
	DATE * newDate = malloc(sizeof(DATE));
	//short int day = atoi(date), month= atoi(date+3), year = atoi(date+6);
	short int day, month, year;
	sscanf(date, "%2hd/%2hd/%4hd", &day, &month, &year);
	*newDate = (DATE) {(char)day,(char)month,year};
	return newDate;
}

int p_getDate(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	if (tempBuffer[0] == '\0') return 0;
	int chars_parsed;
	short int day, month, year;
	if (sscanf(tempBuffer, "%2hd/%2hd/%4hd%n", &day, &month, &year, &chars_parsed) != 3 ||
		tempBuffer[chars_parsed] != '\0' ||
		(day < 1 || day > 31) ||
		(month < 1 || month > 12)
	) {
		return 0;
	}
	*(DATE *)res = (DATE){((char)day),((char)month),year};
	return 1;
}

int p_getDriver(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	if (tempBuffer[0] == '\0') return 0;
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
	char *str = tempBuffer;
	long int val = strtol(tempBuffer, &str, 10);
	if (str == tempBuffer || // string vazia / output invalido
		*str == '.' || // é float e não int
		val <= 0)
	{	
		// printf("distance invalid. string:%s value:%d\n", tempBuffer, (int)val);
		return 0;
	}
	*(short int *)res = (short int)val;
	return 1;
}

int p_getScoreUser(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	char *str = tempBuffer;
	long int val = strtol(tempBuffer, &str, 10);
	if (str == tempBuffer ||
		*str == '.' ||
		val <= 0)
	{
		return 0;
	}
	*(short int *)res = (short int)val;
	return 1;
}

int p_getScoreDriver(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	char *str = tempBuffer;
	long int val = strtol(tempBuffer, &str, 10);
	if (str == tempBuffer ||
		*str == '.' ||
		val <= 0)
	{
		return 0;
	}
	*(short int *)res = (short int)val;
	return 1;
}

// esta função é responsavel por acabar no \n nas rides
int p_getTip(FILE *ptr, void *res) {
	char tempBuffer[BUFF_SIZE];
	writeString(ptr, tempBuffer);
	char *str = tempBuffer;
	float val = strtof(tempBuffer, &str);
	if (str == tempBuffer ||
		val < 0 ||
		val != val) // check for nan
	{
		return 0;
	}
	*(float *)res = val;
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
		return 0;
	}
	*(int *)res = atoi(str);
	free(str);
	return 1;
}

int p_getDriverID(FILE *ptr, void *res) {
	char buff[BUFF_SIZE];
	return safer_writeString(ptr, buff);
}

int p_getComment(FILE *ptr, void *res) {
	while (fgetc(ptr) != '\n');
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

		// free a todos os campos que nao sejam null e que levaram parse até agora
		// coloca o 1º campo que possa levar free a NULL (é assim que vemos que é inválido)
		for (j = 0; j < i; j++) {
			current = array[j];
			if (current.should_free) {
				info = (void **)(field_ptr + current.offset);
				if (*info) free(*info);
				if (!flag) {
					*info = NULL;
					flag = 1;
				}
			}
		}
		// pode acontecer o parse nao ter avançado o suficiente para sequer chegar ao campo que tem de ficar a NULL
		if (!flag) {
			for (; i < (const int)format->len; i++) {
				current = array[i];
				info = (void **)(field_ptr + current.offset);
				if (current.should_free) {
					*info = NULL;
					break;
				}
			}
		}
		
		return 0;
	}
}

void dumpWithFormat(void *data, parse_format *format) {
	int i;
	parse_func_struct *array = format->format_array;
	parse_func_struct current;
	char *field_ptr = ((char*)data);

	// assumo que se puder levar free então é string
	for (i = 0; i < (const int)format->len; i++) {
		current = array[i];
		if (current.should_free) {
			printf("%s\n", *(char **)(field_ptr + current.offset));
		}
	}
}
