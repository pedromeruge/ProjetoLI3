#include "commonParsing.h"
#include <ctype.h>
#include <stdint.h>
#include <execinfo.h>

// buffer ja vem com offset
int p_getID(char *buffer, int *bp, void *res) {
	char *endptr;
	int val = (int)strtol(buffer, &endptr, 10);
	// if buffer[0] == ';'????
	if (endptr == buffer) {
		(*bp) ++;
		return 0;
	}
	*(int *)res = val;
	(*bp) += (endptr - buffer) + 1;
	return 1;
 }

int p_getString(char *buffer, int *bp, void *res) {
	int i;
	for (i = 0; buffer[i] != ';'; i++);
	if (i != 0) {
		// printf("got buffer: %.32s\n", buffer);
		char *__restrict__ str = malloc(i + 1);
		memcpy(str, buffer, sizeof(char) * i);
		str[i] = '\0';
		*(char **)res = str;
		// printf("result string: %s\n", str);
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
	int i;
	(*bp) ++;
	for (i = 0; buffer[i] != ';'; i++);
	if (i != 10) {
		return 0;
	} 

	uint32_t year;
	uint8_t month, day;

	(*bp) += 10; // datas têm SEMPRE 10 de comprimento (acho eu)
	char *endptr1, *endptr2;
	day = (uint8_t)strtol(buffer, &endptr1, 10);
	if (*endptr1 != '/' || day < 1 || day > 31) {
		return 0;
	}
	month = (uint8_t)strtol(endptr1 + 1, &endptr2, 10);
	if (*endptr2 != '/' || month < 1 || month > 12) {
		return 0;
	}
	year = (uint32_t)strtol(endptr2 + 1, &endptr1, 10);
	if (*endptr1 != ';') {
		return 0;
	}
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
		if (strncasecmp("active", buffer, 6) == 0)
		{
			*(unsigned char *)res = ACTIVE;
		}
		else if (strncasecmp("inactive", buffer, 8) == 0)
		{
			*(unsigned char *)res = INACTIVE;
		} else {
			i = 0;
		}
	}
	return i;
}

// checks empty up until;, not \n!!!!!
// OTIMIZAR PARA SER + 12 NO CASO DE SER ID DO DRIVER
int p_checkEmpty(char *buffer, int *bp, void *res) {
	int i;
	for (i = 0; buffer[i] != ';'; i++);
	if (i != 0) {
		// printf("DriverID: %d\n", atoi(buffer));
	}
	// else printf("Invalid\n");
	(*bp) += i + 1;
	return i;
}

int p_checkEmptyNewline(char *buffer, int *bp, void *res) {
	int i;
	for (i = 0; buffer[i] != '\n'; i++);
	(*bp) += i + 1;
	return 1;
}

// OTIMIZAR???????????
int p_getCarClass(char *buffer, int *bp, void *res) {
	int i;
	for (i = 0; buffer[i] != ';'; i++);
	(*bp) += i + 1;
	if (i != 0) {
		//car class é 0, 1 ou 2 (basic/green/premium)
		*(unsigned char *)res = (tolower(buffer[0]) - 97) / 6;
		if (strncasecmp("basic", buffer, 5) != 0 &&
			strncasecmp("green", buffer, 5) != 0 &&
			strncasecmp("premium", buffer, 7) != 0)
		{
			i = 0; 
		}
	}
	return i;
}

int p_getShortPositiveInt(char *buffer, int *bp, void *res) {
	char *endptr;
	short int val = (short int)strtol(buffer, &endptr, 10);
	if (*endptr == '.' ||
		endptr == buffer ||
		val <= 0)
	{
		val = 0;
	}
	*(short int *)res = val;
	(*bp) += endptr - buffer + 1;
	return val;
}

int p_getTip(char *buffer, int *bp, void *res) {
	char *endptr;
	float val = strtof(buffer, &endptr);
	(*bp) += endptr - buffer + 1;
	*(float *)res = val;
	if (endptr == buffer ||
		val < 0 ||
		val != val) // check for nan (???)
	{
		return 0;
	}
	return 1;
}

inline int compDates(Date dateA, Date dateB) {
	// isto transforma o short e 2 chars para 1 int
	return dateA - dateB;
}

// esta função automaticamente dá skip da linha que acabou de levar parse, em caso de erro ou nao
// explicação da (segunda?) função mais maquiavélica do projeto:
// só preenche uma struct de cada vez, de acordo com o format
// o buffer é partilhado de uma call para a outra. so é atualizado quando está vazio (bp >= sp),
// e fazemos sempre update do sp para ter em conta nao fazer parse a uma parte de uma linha mas sim apenas a linhas inteiras
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
	// printf("buffer: %.64s\n%d\n", buffer + *bp, *bp);
	do {
		current = array[i];
		res = current.func(buffer + *bp, bp, field_ptr + current.offset);
		i++;
	} while (i < (const int) format->len && res > 0);

	if (res > 0) { // caso normal
		return 1;
	} else { // caso de erro
		// printf("failed at %d, bp is %d\n", i, *bp);
		// damos free ao que é preciso e metemos o primeiro campo que pode levar free a NULL
		int flag = 0;
		int j;
		void ** info; // 😭😭😭😭😭😭😭

		// skip da linha
		// se for a ultima funçao assumimos que ela ja deu skip ate ao fim da linha e nao fazemos nada
		if (i != format->len) {
			for (j = *bp; buffer[j] != '\n'; j++);
			(*bp) = j + 1;
		}

		// free a todos os campos que nao sejam null e que levaram parse até agora
		// coloca o 1º campo que possa levar free a NULL (é assim que vemos que é inválido)
		for (j = 0; j < i - 1; j++) {
			// printf("In loop, i = %d j = %d\n", i, j); fflush(stdout);
			current = array[j];
			if (current.should_free) {
				info = (void **)(field_ptr + current.offset);
				free(*info);
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
