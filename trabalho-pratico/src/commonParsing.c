#include "commonParsing.h"
#include <ctype.h>
#include <stdint.h>
#include <execinfo.h>

#define IF_EOF(ptr) if(loadString(ptr) == NULL) return -1;

#define BUFF_SIZE 64

// se for vazio return NULL
char *loadString(FILE *ptr)
{
	char sBuffer[128], *str = NULL;
	int i, j, chr;
	for (i = 0; (chr = fgetc(ptr)) != ';'; i++)
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
// -1: EOF
// 0: empty
// 1: correu bem
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

// buffer ja vem com offset
int p_getID(char *buffer, int *bp, void *res) {
	char *endptr;
	int val = (int)strtol(buffer, &endptr, 10);
	// if buffer[0] ?== ';'????
	if (endptr == buffer) {
		(*bp) ++;
		return 0;
	}
	*(int *)res = val;
	(*bp) += (endptr - buffer) + 1;
	return 0;
 }

int p_getString(char *buffer, int *bp, void *res) {
	int i;
	for (i = 0; buffer[i] != ';'; i++);
	if (i != 0) {
		char *str = malloc(i + 1);
		memcpy(str, buffer, sizeof(char) * i);
		str[i] = '\0';
		*(char **)res = str;
	}
	(*bp) += i + 1;
	return i;
}

int p_getGender(char *buffer, int *bp, void *res) {
	unsigned char temp = buffer[0];
	if (temp == ';') {
		(*bp) ++;
		return 0;
	}
	(*bp) += 2;
	*(unsigned char*)res = temp;
	return 1;
}

int p_getDate(char *buffer, int *bp, void *res) {
	(*bp) ++;
	if (buffer[0] == ';') {
		return 0;
	}
	uint32_t year;
	uint8_t month, day;
	char *endptr1, *endptr2;
	day = (uint8_t)strtol(buffer, &endptr1, 10);
	if (*endptr1 != '/' || day < 1 || day > 31) {
		(*bp) += endptr1 - buffer;
		return 0;
	}
	month = (uint8_t)strtol(endptr1 + 1, &endptr2, 10);
	if (*endptr2 != '/' || month < 1 || month > 12) {
		(*bp) += endptr2 - buffer;
		return 0;
	}
	year = (uint32_t)strtol(endptr2 + 1, &endptr1, 10);
	if (*endptr1 != ';') {
		(*bp) += endptr1 - buffer;
		return 0;
	}
	(*bp) += endptr1 - buffer;
	*(Date *)res = (Date) (year << 16 | month << 8 | day);
	return 1;
}

int p_getPayMethod(char *buffer, int *bp, void *res) {
	int i;
	for (i = 0; buffer[i] != ';'; i++);
	(*bp) += i + 1;
	return i;
}

// // esta função só é usada como a última função dos drivers e users
// // logo o check que é usado vai terminar num \n
// MELHORAR ISTO JA QUE ESTOU A FAZER LOOP E ESTOU
int p_getAccountStatus(char *buffer, int *bp, void *res) {
	int i;
	(*bp) ++;
	for (i = 0; buffer[i] != '\n'; i++);
	if (i != 0) {
		(*bp) += i;
		buffer[i] = '\0';
		if (strncasecmp("active", buffer, 6) == 0)
		{
			*(unsigned char *)res = ACTIVE;
		}
		else if (strncasecmp("inactive", buffer, 8) == 0)
		{
			*(unsigned char *)res = INACTIVE;
		} else {
			// buffer[i] = '\n'; // FOR DEBUG
			i = 0;
		}
		// buffer[i] = '\n'; // FOR DEBUG
	}
	return i;
}
// int p_getAccountStatus(FILE *ptr, void *res) {
// 	char str[BUFF_SIZE];
// 	writeString(ptr, str);
// 	if (str[0] == '\0') return 0;
// 	int i;
// 	for (i = 0; str[i]; i++) {
// 		str[i] = tolower(str[i]);
// 	}

// 	unsigned char result;
// 	if (strncmp("active", str, 6) == 0)
// 	{
// 		result = ACTIVE;
// 	}
// 	else if (strncmp("inactive", str, 8) == 0)
// 	{
// 		result = INACTIVE;
// 	} else {
// 		return 0;
// 	}
// 	*(unsigned char *)res = result;
// 	return 1;
// }

// int p_getPayMethod(FILE *ptr, void *res)
// {
// 	char buff[BUFF_SIZE];
// 	writeString(ptr, buff);
// 	if (buff[0] == '\0') return 0;
// 	return 1;
// }

inline int compDates(Date dateA, Date dateB) {
	// isto transforma o short e 2 chars para 1 int
	return dateA - dateB;
}

// int p_getDriver(FILE *ptr, void *res) {
// 	char tempBuffer[BUFF_SIZE];
// 	writeString(ptr, tempBuffer);
// 	if (tempBuffer[0] == '\0') return 0;
// 	*(int *)res = atoi(tempBuffer);
// 	return 1;
// }

// int p_getName(FILE *ptr, void *res) {
// 	*(char **)res = loadString(ptr);
// 	if (*(char **)res == NULL) return 0;
// 	return 1;
// }

// int p_getCity(FILE *ptr, void *res) {
// 	*(char **)res = loadString(ptr);
// 	if (*(char **)res == NULL) return 0;
// 	return 1;
// }

// int p_getDistance(FILE *ptr, void *res) {
// 	char tempBuffer[BUFF_SIZE];
// 	writeString(ptr, tempBuffer);
// 	char *str = tempBuffer;
// 	long int val = strtol(tempBuffer, &str, 10);
// 	if (str == tempBuffer || // string vazia / output invalido
// 		*str == '.' || // é float e não int
// 		val <= 0)
// 	{	
// 		// printf("distance invalid. string:%s value:%d\n", tempBuffer, (int)val);
// 		return 0;
// 	}
// 	*(short int *)res = (short int)val;
// 	return 1;
// }

// int p_getScoreUser(FILE *ptr, void *res) {
// 	char tempBuffer[BUFF_SIZE];
// 	writeString(ptr, tempBuffer);
// 	char *str = tempBuffer;
// 	long int val = strtol(tempBuffer, &str, 10);
// 	if (str == tempBuffer ||
// 		*str == '.' ||
// 		val <= 0)
// 	{
// 		return 0;
// 	}
// 	*(short int *)res = (short int)val;
// 	return 1;
// }

// int p_getScoreDriver(FILE *ptr, void *res) {
// 	char tempBuffer[BUFF_SIZE];
// 	writeString(ptr, tempBuffer);
// 	char *str = tempBuffer;
// 	long int val = strtol(tempBuffer, &str, 10);
// 	if (str == tempBuffer ||
// 		*str == '.' ||
// 		val <= 0)
// 	{
// 		return 0;
// 	}
// 	*(short int *)res = (short int)val;
// 	return 1;
// }

// // esta função é responsavel por acabar no \n nas rides
// int p_getTip(FILE *ptr, void *res) {
// 	char tempBuffer[BUFF_SIZE];
// 	writeString(ptr, tempBuffer);
// 	char *str = tempBuffer;
// 	float val = strtof(tempBuffer, &str);
// 	if (str == tempBuffer ||
// 		val < 0 ||
// 		val != val) // check for nan
// 	{
// 		return 0;
// 	}
// 	*(float *)res = val;
// 	return 1;
// }

// int p_getCarClass(FILE *ptr, void *res) {
// 	// *(unsigned char *)res = (fgetc(ptr) - 97) / 6;
// 	// while (fgetc(ptr) != ';');
// 	char str[BUFF_SIZE];
// 	writeString(ptr, str);
// 	if (str[0] == '\0') return 0;
// 	int i;
// 	for(i = 0; str[i]; i++){
// 		str[i] = tolower(str[i]);
// 	}
// 	//car class é 0, 1 ou 2 (basic/green/premium)
// 	*(unsigned char *)res = (str[0] - 97) / 6;
// 	if (strncmp("basic", str, 5) == 0 ||
// 		strncmp("green", str, 5) == 0 ||
// 		strncmp("premium", str, 7) == 0)
// 	{
// 			return 1;
// 	}
// 	// else
// 	return 0;
// }

// int p_getLicensePlate(FILE *ptr, void *res) {
// 	*(char **)res = loadString(ptr);
// 	if (*(char **)res == NULL) return 0;
// 	return 1;
// }

// int p_getID(FILE *ptr, void *res) {
// 	char buff[BUFF_SIZE];
// 	int ret = safer_writeString(ptr, buff);
// 	if (ret == 1) {
// 		*(int *)res = atoi(buff);
// 	}
// 	return ret;
// }

// int p_getDriverID(FILE *ptr, void *res) {
// 	char buff[BUFF_SIZE];
// 	return safer_writeString(ptr, buff);
// }

// int p_getComment(FILE *ptr, void *res) {
// 	while (fgetc(ptr) != '\n');
// 	return 1;
// }

// esta função automaticamente dá skip da linha que acabou de levar parse, em caso de erro ou nao
// explicação da (segunda?) função mais maquiavélica do projeto:
// só preenche uma struct de cada vez, de acordo com o format
// o buffer é partilhado de uma call para a outra. so é atualizado quando está vazio (bp == sp),
// e fazemos sempre update do sp para ter em conta nao fazer parse a um parte de uma linha mas sim apenas a linhas inteiras
// o bp é atualizado pelas funçoes de parse, que lhe somam o numero de caracteres "avançados", incluindo ';' e etc
// o tamanho do buffer é assumido ser SIZE_OF_READ_BUFFER
int parse_with_format(FILE *ptr, void *data, const parse_format *format, int *bp, int *sp, char *buffer) {
	if (*bp >= *sp) {
		int chars = fread(buffer, 1, SIZE_OF_READ_BUFFER, ptr) - 1;
		if (chars == -1) return -1; // EOF
		*bp = 0;
		if (buffer[chars] != '\n') {
			int i = chars;
			do {
				chars --;
			} while (buffer[chars] != '\n');
			fseek(ptr, chars - i, SEEK_CUR);
			*sp = chars;
		} else {
			*sp = chars; // conta com o \n do fim!!!
		}
	}

	int i = 0, res;
	parse_func_struct *array = format->format_array;
	parse_func_struct current;
	char *field_ptr = ((char*)data);
	// res =
	// 0: erro
	// >0 : correu bem
	do {
		current = array[i];
		res = current.func(buffer + *bp, bp, field_ptr + current.offset);
		i++;
	} while (i < (const int) format->len && res > 0);

	if (i == format->len && res > 0) { // caso normal
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

Date atoDate(const char *str) {
	// DD/MM/YYYY
	int day = atoi(str), month = atoi(str + 3), year = atoi(str + 6);
	return (uint32_t)(((uint32_t) year) << 16 | ((uint8_t) month) << 8 | (uint8_t) day);
}
