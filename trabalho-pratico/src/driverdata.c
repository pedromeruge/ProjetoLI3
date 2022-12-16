#include "driverdata.h"

#define DRIVER_STR_BUFF 32

#define N_OF_FIELDS 8

struct DriverStruct
{
	// id está subentendido pela posição
	char *name;
	char *birthdate;
	unsigned char gender;
	// estes podem todos ser mudados para alguma coisa melhor
	unsigned char carClass; // é usado o primeiro char da palavra para converter em 0, 1 ou 2
	char *licensePlate;
	char *city;
	char *accountCreation;
	unsigned char status;
};

typedef struct {
	int len;
	DriverStruct array[SIZE]; // já sabemos que vai levar malloc de SIZE e já, e assim usamos g_ptr_array
} SecondaryDriverArray;

struct DriverData {
	GPtrArray* driverArray;
};

struct parse_func_struct {
	parse_func *func;
	size_t offset;
};

void freeDriverPtrArray(void * data);

SecondaryDriverArray *getDrivers(FILE *ptr, parse_format *format)
{
	int i, tempchr, count, chr, id_size;
	// char *name;

	SecondaryDriverArray *resArray = malloc(sizeof(SecondaryDriverArray));
	DriverStruct * driverStructArray = resArray->array;

	for (i = count = 0; i < SIZE; i++, count++)
	{
		for (id_size = 0; (chr = fgetc(ptr)) != ';' && chr != EOF; id_size++); // && chr != -1); // skip id
		
		if (chr == EOF) {
			break; //break feio???????
		}

		if (id_size != 0) {
			parse_with_format(ptr, (void *)&driverStructArray[i], format);
		}

		// avaçar até proxima linha
		while ((tempchr = fgetc(ptr)) != '\n'); // && (tempchr != -1));
	}

	resArray->len = i;// - 1 ????
	if (chr == EOF && i == 0) {
			free(resArray);
			resArray = NULL; //fim do ficheiro e não recebemos info nenhuma
	}
	
	// for (i = 0; i < count; i++) {
	// 	printf("%s %s %d %c %s %s %s %d\n", driverStructArray[i].name,
	// 	driverStructArray[i].birthdate,
	// 	driverStructArray[i].gender,
	// 	driverStructArray[i].carClass,
	// 	driverStructArray[i].licensePlate,
	// 	driverStructArray[i].city,
	// 	driverStructArray[i].accountCreation,
	// 	driverStructArray[i].status);
	// }

	return resArray;
}

DriverData * getDriverData(FILE *ptr)
{
	DriverData * newDriverData = malloc(sizeof(DriverData));
	GPtrArray * driverarray = g_ptr_array_new_with_free_func(freeDriverPtrArray);
	SecondaryDriverArray *secondaryArray;

	parse_format format;

	parse_func_struct format_array[N_OF_FIELDS] = {
		{ p_getName, offsetof(DriverStruct, name), 1, },
		{ p_getDate, offsetof(DriverStruct, birthdate), 1, },
		{ p_getGender, offsetof(DriverStruct, gender), 0, },
		{ p_getCarClass, offsetof(DriverStruct, carClass), 0, },
		{ p_getLicensePlate, offsetof(DriverStruct, licensePlate), 1, },
		{ p_getCity,offsetof(DriverStruct, city), 1, },
		{ p_getDate, offsetof(DriverStruct, accountCreation), 1, },
		{ p_getAccountStatus, offsetof(DriverStruct, status), 0, },
	};

	// que confusao nao sei fazer nomes
	format.format_array = format_array;
	format.len = N_OF_FIELDS;

	while (fgetc(ptr) != '\n')
		; // avançar a primeira linha (tbm podia ser um seek hardcoded)
	secondaryArray = getDrivers(ptr, &format);


	while (secondaryArray != NULL) {
		g_ptr_array_add(driverarray, secondaryArray);
		secondaryArray = getDrivers(ptr, &format);
	}

	newDriverData->driverArray = driverarray;


	int num = (driverarray->len - 1) * SIZE;
	secondaryArray = g_ptr_array_index(driverarray, driverarray->len - 1);
	num += secondaryArray->len;
	printf("Total number of drivers: %d\n", num);

	return newDriverData;
}

void freeDriverData(DriverData * data)
{
	g_ptr_array_free(data->driverArray, TRUE);
	free(data);
}

void freeDriverPtrArray(void * data) {
	SecondaryDriverArray *secondaryArray = (SecondaryDriverArray *)data;
	DriverStruct * array = secondaryArray->array;
	
	int i;
	DriverStruct block;
	for (i = 0; i < (const int)secondaryArray->len; i++) {
		block = array[i];
		free(block.name);
		free(block.birthdate);
		free(block.licensePlate);
		free(block.city);
		free(block.accountCreation);
	}
	free(secondaryArray);
}

// devolve a struct(dados) associada ao driver número i
DriverStruct *getDriverPtrByID(DriverData * data, guint ID)
{
	// no bounds checking
	ID -= 1;
	int i = ID / SIZE;
	SecondaryDriverArray *secondaryArray = g_ptr_array_index(data->driverArray, i);
	DriverStruct * result = &(secondaryArray->array[ID - SIZE * i]);
	return result;
}

char *getDriverName(DriverStruct *driver)
{
	return strndup(driver->name, DRIVER_STR_BUFF);
}

char *getDriverBirthdate(DriverStruct *driver)
{
	return strndup(driver->birthdate, DRIVER_STR_BUFF);
}

unsigned char getDriverGender(DriverStruct *driver)
{
	return (driver->gender);
}

unsigned char getDriverCar(DriverStruct *driver)
{
	return (driver->carClass);
}
char *getDriverPlate(DriverStruct *driver)
{
	return strndup(driver->licensePlate, DRIVER_STR_BUFF);
}

char *getDriverCity(DriverStruct *driver)
{
	return strndup(driver->city, DRIVER_STR_BUFF);
}

char *getDriverAccCreation(DriverStruct *driver)
{
	return strndup(driver->accountCreation, DRIVER_STR_BUFF);
}

unsigned char getDriverStatus(DriverStruct *driver)
{
	return (driver->status);
}
