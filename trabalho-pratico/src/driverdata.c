#include "driverdata.h"

#define DRIVER_STR_BUFF 32

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

void freeDriverPtrArray(void * data);

SecondaryDriverArray *getDrivers(FILE *ptr)
{
	int i, tempchr, count, chr;
	// char *name;

	SecondaryDriverArray *resArray = malloc(sizeof(SecondaryDriverArray));
	DriverStruct * driverStructArray = resArray->array;

	for (i = count = 0; i < SIZE; i++, count++)
	{
		while ((chr = fgetc(ptr)) != ';' && chr != EOF); // && chr != -1); // skip id
		if (chr == EOF) {
			break; //break feio
		}
		// name = loadString(ptr);
		driverStructArray[i].name = loadString(ptr);
		// if (name == NULL) break;
		driverStructArray[i].birthdate = loadString(ptr);
		driverStructArray[i].gender = fgetc(ptr);
		fseek(ptr, 1, SEEK_CUR);
		driverStructArray[i].carClass = (fgetc(ptr) - 97) / 6;
		while (fgetc(ptr) != ';');
		driverStructArray[i].licensePlate = loadString(ptr);
		driverStructArray[i].city = loadString(ptr);
		driverStructArray[i].accountCreation = loadString(ptr);
		driverStructArray[i].status = getAccountStatus(ptr);

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

	while (fgetc(ptr) != '\n')
		; // avançar a primeira linha (tbm podia ser um seek hardcoded)
	secondaryArray = getDrivers(ptr);
	while (secondaryArray != NULL) {
		g_ptr_array_add(driverarray, secondaryArray);
		secondaryArray = getDrivers(ptr);
	}

	newDriverData->driverArray = driverarray;
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
