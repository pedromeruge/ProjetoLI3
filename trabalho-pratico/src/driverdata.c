#include "driverdata.h"

#define DRIVER_STR_BUFF 32

#define N_OF_FIELDS 9

#define SIZE (1 << 14)

#define DRIVER_IS_VALID(driver) (driver->name != NULL)

struct DriverStruct
{
	// id está subentendido pela posição
	char *name;
	Date birthdate;
	unsigned char gender;
	unsigned char carClass; // é usado o primeiro char da palavra para converter em 0, 1 ou 2
	char *licensePlate;
	// char *city; // não usados
	Date accountCreation;
	unsigned char status;
};

typedef struct {
	int len;
	DriverStruct array[SIZE]; // sub-arrays levam malloc de tamanho SIZE fixo
} SecondaryDriverArray;

struct DriverData {
	GPtrArray* driverArray;
	unsigned int numberOfDrivers;
};

struct parse_func_struct {
	parse_func *func;
	size_t offset;
};

void freeDriversPtrArray(void * data);

SecondaryDriverArray *getDrivers(FILE *ptr, const parse_format *format, int *invalid, int *bp, int *sp, char *buffer)
{
	int i, res;

	SecondaryDriverArray *resArray = malloc(sizeof(SecondaryDriverArray));
	DriverStruct * driverStructArray = resArray->array;
	// int old , new;
	for (i = 0; i < SIZE; i++)
	{
		// old = atoi(buffer + *bp);
		if ((res = parse_with_format(ptr, (void *)&driverStructArray[i], format, bp, sp, buffer)) == 0)
		{
			(*invalid)++;
		} else if (res == -1) { //EOF
			if (i == 0) {
				free(resArray);
				resArray = NULL;
			}
			break;
		}
		// new = atoi(buffer + *bp);
		// printf("%d %d\n", old, new);
		// if (new - old != 1) printf("%d %d\n", old, new);
	}

	if (resArray != NULL) resArray->len = i;

	return resArray;
}

DriverData * getDriverData(FILE *ptr, char *buffer)
{
	DriverData * newDriverData = malloc(sizeof(DriverData));
	GPtrArray * driverarray = g_ptr_array_new_with_free_func(freeDriversPtrArray);
	SecondaryDriverArray *secondaryArray;

	parse_format format;

	parse_func_struct format_array[N_OF_FIELDS] = {
		{ p_checkEmpty, 0, 0 }, // os dados disto nunca sao escritos, é só para ver se é vazio ou não
		{ p_getString, offsetof(DriverStruct, name), 1, },
		{ p_getDate, offsetof(DriverStruct, birthdate), 0, },
		{ p_getGender, offsetof(DriverStruct, gender), 0, },
		{ p_getCarClass, offsetof(DriverStruct, carClass), 0, },
		{ p_getString, offsetof(DriverStruct, licensePlate), 1, },
		{ p_checkEmpty, 0, 0}, // os dados disto nunca sao escritos, é só para ver se é vazio ou não
		{ p_getDate, offsetof(DriverStruct, accountCreation), 0, },
		{ p_getAccountStatus, offsetof(DriverStruct, status), 0, },
	};

	int bp = 0, sp = 0;

	// que confusao nao sei fazer nomes
	format.format_array = format_array;
	format.len = N_OF_FIELDS;

	int invalid = 0;
	while (fgetc(ptr) != '\n'); // avançar a primeira linha (tbm podia ser um seek hardcoded)
	secondaryArray = getDrivers(ptr, &format, &invalid, &bp, &sp, buffer);

	while (secondaryArray != NULL) {
		g_ptr_array_add(driverarray, secondaryArray);
		secondaryArray = getDrivers(ptr, &format, &invalid, &bp, &sp, buffer);
	}

	newDriverData->driverArray = driverarray;

	int num = (driverarray->len - 1) * SIZE;
	secondaryArray = g_ptr_array_index(driverarray, driverarray->len - 1);
	num += secondaryArray->len;
	printf("Total number of drivers: %d\nNumber of invalid drivers: %d\n", num, invalid);
	newDriverData->numberOfDrivers = num;

	return newDriverData;
}

void freeDriverData(DriverData * data)
{
	g_ptr_array_free(data->driverArray, TRUE);
	free(data);
}

void freeDriversPtrArray(void * data) {
	SecondaryDriverArray *secondaryArray = (SecondaryDriverArray *)data;
	DriverStruct * array = secondaryArray->array;
	
	int i;
	DriverStruct *block;
	for (i = 0; i < (const int)secondaryArray->len; i++) {
		block = &array[i];
		if (DRIVER_IS_VALID(block)) {
			free(block->name);
			//free(&block->birthdate);
			free(block->licensePlate);
			//free(block->city);
			//free(&block->accountCreation);
		}
	}
	free(secondaryArray);
}

// devolve a struct(dados) associada ao driver número i
DriverStruct *getDriverPtrByID(const DriverData * data, guint ID)
{
	// no bounds checking
	GPtrArray *array = data->driverArray;
	ID -= 1;
	guint i = ID / SIZE;
	// deixou de ser checked aqui
	// if (i > array->len) return NULL;
	SecondaryDriverArray *secondaryArray = g_ptr_array_index(array, i);
	DriverStruct * result = &(secondaryArray->array[ID & (SIZE - 1)]);
	return DRIVER_IS_VALID(result) ? result : NULL;
}

// devolve o número de drivers no ficheiro de input; função necessária para criar arrays sobre drivers, no ridesData.c
inline int getNumberOfDrivers (const DriverData * driverData) {
	return driverData->numberOfDrivers;
}

// 1 se ultrapassar bounds
inline int testDriverBounds(const DriverData *data, guint ID) {
	return (ID - 1 > data->numberOfDrivers) ? 1 : 0;
}

inline char *getDriverName(const DriverStruct *driver)
{
	return strndup(driver->name, DRIVER_STR_BUFF);
}

//ver modularidade depois!
inline Date getDriverBirthdate(const DriverStruct *driver)
{
	return driver->birthdate;
}

inline unsigned char getDriverGender(const DriverStruct *driver)
{
	return (driver->gender);
}

inline unsigned char getDriverCar(const DriverStruct *driver)
{
	return (driver->carClass);
}
inline char *getDriverPlate(const DriverStruct *driver)
{
	return strndup(driver->licensePlate, DRIVER_STR_BUFF);
}

// inline char *getDriverCity(const DriverStruct *driver)
// {
// 	return strndup(driver->city, DRIVER_STR_BUFF);
// }

//ver modularidade depois!
inline Date getDriverAccCreation(const DriverStruct *driver)
{
	return driver->accountCreation;
}

inline unsigned char getDriverStatus(const DriverStruct *driver)
{
	return (driver->status);
}

inline int driverIsValid(const DriverStruct *driver) {
	return (driver != NULL && DRIVER_IS_VALID(driver));
}
