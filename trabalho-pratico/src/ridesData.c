#include "ridesData.h"

#define RIDE_STR_BUFF 32

struct RidesStruct
{
	char *date;
	short int driver;
	char *user;
	char *city;
	short int distance,
		score_u,
		score_d;
	float tip;
	char *comment;
};

struct CityRides
{
	GPtrArray *array;
}; 

struct ridesByDriver {
	GPtrArray * ridesArray;
};

//informações de todas a rides para um user
struct driverRatingInfo {
    void * ratingChart; // void * porque vai guardar um array de ratings de 1 a 5 e distância viajada, e mais tarde é convertido para um (double *) do valor médio desses ratings
    double tips; // inicialmente guarda número de tips, depois guarda o total_auferido
    char * mostRecRideDate; // ride mais decente
    short int  * ridesNumber, // guardar [número de rides, total viajado]
    		   driverNumber; // talvez meter em int o valor, ocupa menos espaço com char?
};

//função que guarda arrays criados e acedidos recorrentemente, para reduzir tempo de processamento
typedef struct querySavedData {
	GPtrArray * driverRatingArray; // array da query 2
} querySavedData;

struct RidesData {
	RidesStruct **ridesArray;
	GHashTable *cityTable;
	GPtrArray * driverInfoArray;
	querySavedData * savedData;
};

void freeArray(void *data);
gint compareRidesByDate(gconstpointer, gconstpointer);
void *sortCity(void *);
RidesStruct *getRides(FILE *, GHashTable *, GPtrArray *);

GPtrArray * addDriverInfo(GPtrArray *,RidesStruct *);
driverRatingInfo * newDriverInfo (RidesStruct *);
driverRatingInfo * appendDriverInfo (driverRatingInfo *, RidesStruct *);
GPtrArray * getPresentableValues (GPtrArray *);
driverRatingInfo * sumValues (driverRatingInfo *);
driverRatingInfo * newOpaqueDriverInfo (int);
void freeRidesRating(void *);


void *sortCity(void *data)
{
	GPtrArray *array = (GPtrArray *)data;
	g_ptr_array_sort(array, compareRidesByDate);
	// g_thread_exit(NULL);
	return NULL;
}

RidesData * getRidesData(FILE *ptr)
{
	int numberOfDrivers = DRIVER_ARR_SIZE * SIZE, i;
	//inicializar as estruturas de dados relacionadas com as rides
	RidesStruct **ridesData = malloc(RIDES_ARR_SIZE * sizeof(RidesStruct *));

	GHashTable *cityTable = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, freeArray); // keys levam malloc do array normal, nao vou dar free aqui;
	
	GPtrArray * driverInfoArray = g_ptr_array_new_full(numberOfDrivers, freeRidesRating);
	g_ptr_array_set_size(driverInfoArray,numberOfDrivers); // o tamanho do array tem de ser definido, apesar de ja ter sido alocado o espaço para o tamanho necessário; o array tem de ser inicializado a NULL para todos os pointers, feito por g_ptr_array_set_size

	while (fgetc(ptr) != '\n')
		; // avançar a primeira linha (tbm podia ser um seek hardcoded)
	for (i = 0; i < RIDES_ARR_SIZE; i++)
		ridesData[i] = getRides(ptr, cityTable,driverInfoArray);

	driverInfoArray = getPresentableValues(driverInfoArray);

	RidesData *data = malloc(sizeof(RidesData));
	data->ridesArray = ridesData; // array com input do ficheiro
	data->cityTable = cityTable; // array com rides ordenandas por cidade
	data->driverInfoArray = driverInfoArray; // array com informação de drivers resumida
	data->savedData = malloc(sizeof(querySavedData)); // estrutura em que se guarda novos array criados nas queries
	data->savedData->driverRatingArray = NULL;

	// tentei usar GThreadPool, desisti
	guint num_keys = g_hash_table_size(cityTable);
	GThread *threads[num_keys];

	num_keys --; // a city final é feita pela main thread

	GHashTableIter iter;
	gpointer value;
	g_hash_table_iter_init (&iter, cityTable);
	for (i = 0; g_hash_table_iter_next (&iter, NULL, &value) && i < (int)num_keys; i++)
	{
		threads[i] = g_thread_new(NULL, sortCity, value);
		// sortCity(&args[i]);
	}
	// mudei isto, a main thread ficava sem fazer nada, assim faz uma das cities
	g_hash_table_iter_next (&iter, NULL, &value);
	sortCity(value);

	for (i = 0; i < (int)num_keys; i++) {
		g_thread_join(threads[i]);
	}

	return data;
}

RidesStruct *getRides(FILE *ptr, GHashTable *cityTable, GPtrArray * driverInfoArray)
{
	int i, count, chr;
	char tempBuffer[16], *city;
	GPtrArray *array;
	RidesStruct *ridesStructArray = malloc(SIZE * sizeof(RidesStruct)), *temp;
	for (i = count = 0; i < SIZE; i++, count++)
	{
		while ((chr = fgetc(ptr)) != ';')
			; // && chr != -1); // skip id
		ridesStructArray[i].date = loadString(ptr);
		writeString(ptr, tempBuffer);
		ridesStructArray[i].driver = (short)atoi(tempBuffer);
		ridesStructArray[i].user = loadString(ptr);
		city = loadString(ptr);
		ridesStructArray[i].city = city;
		writeString(ptr, tempBuffer);
		ridesStructArray[i].distance = (short)atoi(tempBuffer);
		writeString(ptr, tempBuffer);
		ridesStructArray[i].score_u = (short)atoi(tempBuffer);
		writeString(ptr, tempBuffer);
		ridesStructArray[i].score_d = (short)atoi(tempBuffer);
		writeString(ptr, tempBuffer);
		ridesStructArray[i].tip = atof(tempBuffer);
		ridesStructArray[i].comment = loadString(ptr); // e se for null?????????????????

		temp = &(ridesStructArray[i]);
		// check if city is not already in hash table
		if ((array = g_hash_table_lookup(cityTable, city)) == NULL)
		{
			// if not, insert
			array = g_ptr_array_sized_new(200000); // +/- 195000 por cidade
			g_ptr_array_add(array, temp);
			g_hash_table_insert(cityTable, city, array);
		}
		else
		{
			// if yes, append to all the other data
			g_ptr_array_add(array, temp);
			// printf("Adding to %s a ride in %s\n", city, temp->city);
		}
		driverInfoArray = addDriverInfo(driverInfoArray, ridesStructArray + i);
	}

	// for (i = 0; i < count; i++) {
	// 	printf("%s %hd %s %s %hd %hd %hd %.1f %s\n", ridesStructArray[i].date,
	// 	ridesStructArray[i].driver,
	// 	ridesStructArray[i].user,
	// 	ridesStructArray[i].city,
	// 	ridesStructArray[i].distance,
	// 	ridesStructArray[i].score_u,
	// 	ridesStructArray[i].score_d,
	// 	ridesStructArray[i].tip,
	// 	ridesStructArray[i].comment);
	// }

	return ridesStructArray;
}

GPtrArray *addDriverInfo(GPtrArray *driverInfoArray, RidesStruct *currentRide)
{
	driverRatingInfo *newStruct = NULL, // pointer para struct que vai ser guardada em cada pos do array
		*currentArrayStruct = NULL;
	short int driverNumber = currentRide->driver;
	currentArrayStruct = (driverRatingInfo *)g_ptr_array_index(driverInfoArray, driverNumber - 1);
	if (currentArrayStruct == NULL)
	{ // verifica se no local atual ainda n existe informação de um driver, e se este tem o estado ativo
		newStruct = newDriverInfo(currentRide);
		g_ptr_array_index(driverInfoArray, driverNumber - 1) = newStruct;
	}
	else
	{ // se já existir informação de um driver (de rides prévias);
		appendDriverInfo(currentArrayStruct, currentRide);
	}
	return (driverInfoArray);
}

// criar uma nova struct com informação de um rider ( na primeira vez que aparece nas rides)
driverRatingInfo *newDriverInfo(RidesStruct *currentRide)
{

	driverRatingInfo *new = malloc(sizeof(driverRatingInfo));

	new->ratingChart = calloc(5, sizeof(unsigned int));
	unsigned int *ratings = (unsigned int *)new->ratingChart;
	ratings[(currentRide->score_d) - 1]++;
	new->ridesNumber = calloc(2, sizeof(short int)); // não é utilizada até ao final do ciclo do addRides
	short int *ridesTracker = (short int *)new->ridesNumber;
	ridesTracker[1] = currentRide->distance;
	new->tips = currentRide->tip;
	new->mostRecRideDate = strdup(currentRide->date); // devo copiar a string para a nova struct, para não haver dependências no futuro??
	new->driverNumber = currentRide->driver;
    return new;
}

driverRatingInfo *appendDriverInfo(driverRatingInfo *currentArrayStruct, RidesStruct *currentRide)
{

	char *rideDate = currentRide->date;

	currentArrayStruct->tips += currentRide->tip;

	unsigned int *ratings = (unsigned int *)currentArrayStruct->ratingChart;
	ratings[(currentRide->score_d) - 1]++; // dependendo da avaliação na ride atual, no array ratingChart , incrementa em 1 o número da avalições de valor 1,2,3,4 ou 5.

	short int *ridesTracker = (short int *)currentArrayStruct->ridesNumber;
	ridesTracker[1] += currentRide->distance;

	if (compDates(rideDate, currentArrayStruct->mostRecRideDate) >= 0)
	{
		free(currentArrayStruct->mostRecRideDate);
		currentArrayStruct->mostRecRideDate = strdup(rideDate);
	}
    return (currentArrayStruct);
}

GPtrArray *getPresentableValues(GPtrArray *driverInfoArray)
{
	int i;
    driverRatingInfo * currentArrayStruct = NULL,
                     * newStruct = NULL;
    gint arraySize = driverInfoArray->len;
	for (i = 0; i < arraySize; i++)
	{
		currentArrayStruct = (driverRatingInfo *)g_ptr_array_index(driverInfoArray, i);
		if (currentArrayStruct)
		{
			sumValues(currentArrayStruct);
		}
		else
		{ // caso não haja informação do driver em rides, cria uma struct opaca, para não dar erro em funções de sort
			newStruct = newOpaqueDriverInfo(i + 1);
			g_ptr_array_index(driverInfoArray, i) = newStruct;
		}
	}
	return driverInfoArray;
}

driverRatingInfo * sumValues (driverRatingInfo * currentArrayStruct) {
    unsigned int * ratings = (unsigned int *) currentArrayStruct->ratingChart,
                   numRides = 0, j;
    double avgRating = 0;

	for (j = 0; j < 5; j++)
	{
		avgRating += ratings[j] * (j + 1);
		numRides += ratings[j];
	}
	avgRating /= numRides;
	free(ratings);
	currentArrayStruct->ridesNumber[0] = numRides;
	currentArrayStruct->ratingChart = malloc(sizeof(double));
	*(double *)(currentArrayStruct->ratingChart) = avgRating;
	return (currentArrayStruct);
}

driverRatingInfo *newOpaqueDriverInfo(int driverNumber)
{
	driverRatingInfo *newStruct = malloc(sizeof(driverRatingInfo));
	newStruct->ratingChart = malloc(sizeof(double)); // precisa de ser double, para na função de comparação não dar problemas
	*(double *)(newStruct->ratingChart) = 0;
	newStruct->driverNumber = (short int)driverNumber;
	newStruct->mostRecRideDate = NULL; // não será usado
	newStruct->tips = 0;
	newStruct->ridesNumber = 0;
	return (newStruct);
}

gint compareRidesByDate(gconstpointer a, gconstpointer b)
{
	gint res;
	char *dateA = (*(RidesStruct **)a)->date, *dateB = (*(RidesStruct **)b)->date;
	// DD/MM/YYYY
	if ((res = strncmp(dateA + 6, dateB + 6, 4)) != 0)
	{
		return res;
	}
	else if ((res = strncmp(dateA + 3, dateB + 3, 2)) != 0)
	{
		return res;
	}
	else
	{
		return (strncmp(dateA, dateB, 2));
	}
}

void freeRidesData(RidesData *data)
{
	RidesData *dataStruct = data;
	RidesStruct **ridesData = dataStruct->ridesArray;
	// free de rides por cidade
	GHashTable *table = dataStruct->cityTable;
	g_hash_table_destroy(table);
	// free do array com info resumida de drivers
	GPtrArray *driverInfoArray = dataStruct->driverInfoArray;
	g_ptr_array_free(driverInfoArray, TRUE);
	// free de novos arrays criados nas queries
	querySavedData * savedData = dataStruct->savedData;
	g_ptr_array_free(savedData->driverRatingArray,TRUE); // certo??

	int i, j;
	RidesStruct *segment, block;
	for (i = 0; i < RIDES_ARR_SIZE; i++)
	{
		segment = ridesData[i];
		for (j = 0; j < SIZE; j++)
		{
			block = segment[j];
			free(block.date);
			free(block.user);
			free(block.city);
			free(block.comment);
		}
		free(segment);
	}
	free(ridesData);
	free(dataStruct);
}

void freeRidesRating(void *drivesRating)
{
	driverRatingInfo *currentArrayStruct = (driverRatingInfo *)drivesRating;
	free(currentArrayStruct->ratingChart);
	free(currentArrayStruct->mostRecRideDate);
	free(currentArrayStruct->ridesNumber);
	free(currentArrayStruct);
}

void freeArray(void *data)
{
	GPtrArray *array = (GPtrArray *)data;
	g_ptr_array_free(array, TRUE);
}

//funções relativas a rides ordenadas por driver
const ridesByDriver * getRidesByDriver(const RidesData * ridesData) {// responsabilidade da caller function dar free
	ridesByDriver * driverInfoArray = malloc(sizeof(ridesByDriver));
	driverInfoArray->ridesArray = ridesData->driverInfoArray;
	return driverInfoArray;
}
/*
//// VAI PASSAR A SER A FUNÇÃO PRINCIPAL TALVEZ??? PODE SER USADO NO CONJUNTO TOTAL DAS RIDES DO FICHEIRO, OU EM PARTE : SÓ DENTRO DE UMA CIDADE
const ridesByDriver * getRidesByDriverInCity(const CityRides * cityRides) {
	gint i, numberOfRides = ridesInCity->len;
	GPtrArray * ridesInCity = cityRides->array;
	GPtrArray * driverInfoArray = g_ptr_array_new_full(numberOfDrivers, freeRidesRating);
	g_ptr_array_set_size(driverInfoArray,numberOfDrivers); // o tamanho do array tem de ser definido, apesar de ja ter sido alocado o espaço para o tamanho necessário; o array tem de ser inicializado a NULL para todos os pointers, feito por g_ptr_array_set_size

	for (i=0;i<numberOfRides;i++) {

	}
}
*/
const driverRatingInfo *getDriverInfo(const ridesByDriver *ridesByDriver, guint id)
{
	driverRatingInfo *currentArrayStruct = (driverRatingInfo *)g_ptr_array_index(ridesByDriver->ridesArray, id - 1);
	return (currentArrayStruct);
}

short int getridesByDriverArraySize(const ridesByDriver *driverInfoArray)
{
	return (driverInfoArray->ridesArray->len);
}

gint sort_byRatings (gconstpointer a, gconstpointer b) {
    const driverRatingInfo * driver1 = *(driverRatingInfo **) a;
    const driverRatingInfo * driver2 = *(driverRatingInfo **) b;
    double drv1Rating,drv2Rating;
	drv1Rating = *(double *) driver1->ratingChart;
	drv2Rating = *(double *) driver2->ratingChart;
    double diff = drv1Rating - drv2Rating;	
    gint result = 0;
    if (diff > 0) result = 1;
    else if (diff <0) result = -1;
    else if (!result && drv1Rating) {  // previne comparações entre nodos de riders com rating 0 (não apareciam nas rides)
        result = compDates(driver1->mostRecRideDate,driver2->mostRecRideDate);
        if (!result) 
            result = driver1->driverNumber - driver2->driverNumber;
    }
    return result;
}

const ridesByDriver * qSortArray(const ridesByDriver * oldRidesByDriver, RidesData * ridesData, gint (*sort_byRatings)(gconstpointer a, gconstpointer b)) {
	GPtrArray * driverInfoArray = oldRidesByDriver->ridesArray; // array de input
	GPtrArray * driverRatingArray = ridesData->savedData->driverRatingArray; // array atualmente guardado em ridesData (pode ainda não existir -> NULL)
	if (driverRatingArray == NULL) {
		//driverRatingArray = g_ptr_array_new(); // é preciso para o resultado do copy??
		driverRatingArray = g_ptr_array_copy (driverInfoArray,NULL,NULL); // pointer array oco, só os pointers, não copia o conteúdo todo
		g_ptr_array_set_free_func(driverRatingArray,NULL); // o novo array copia a função de free do anterior, mas como é um pointer array oco, não dá free dos elementos para que aponta
		g_ptr_array_sort(driverRatingArray, *sort_byRatings);
		ridesData->savedData->driverRatingArray = driverRatingArray;
	}
	// criar novo wrapper para o array
	ridesByDriver * newRidesByDriver  = malloc(sizeof(ridesByDriver));
	newRidesByDriver->ridesArray = driverRatingArray;

	free((ridesByDriver *) oldRidesByDriver); // free do wrapper anterior, preserva-se o array, pois é usado noutras funções
	return newRidesByDriver;
}

double getDriverAvgRating(const driverRatingInfo *currentArrayStruct)
{
	return (*(double *)currentArrayStruct->ratingChart);
}

double getDriverTipsTotal(const driverRatingInfo *currentArrayStruct)
{
	return (currentArrayStruct->tips);
}

char *getDriverMostRecRideDate(const driverRatingInfo *currentArrayStruct)
{
	return strndup(currentArrayStruct->mostRecRideDate, RIDE_STR_BUFF);
}

short int getDriverRidesNumber(const driverRatingInfo *currentArrayStruct)
{
	return (currentArrayStruct->ridesNumber[0]);
}

short int getDriverDistTraveled(const driverRatingInfo *currentArrayStruct)
{
	return (currentArrayStruct->ridesNumber[1]);
}

short int getDriverNumber(const driverRatingInfo *currentArrayStruct)
{
	return currentArrayStruct->driverNumber;
}

// funções relativas a rides ordenadas por cidade
CityRides *getRidesByCity(RidesData *data, char *city) // responsabilidade da caller function dar free
{
	CityRides *resultRides = malloc(sizeof(CityRides));
	resultRides->array = g_hash_table_lookup(data->cityTable, city);
	return resultRides;
}

guint getNumberOfCityRides(CityRides *rides)
{
	return rides->array->len;
}

RidesStruct *getCityRidesByIndex(CityRides *rides, guint ID)
{
	return (RidesStruct *)g_ptr_array_index(rides->array, (int)ID);
}

void iterateOverCities(RidesData *rides, void *data, void (*iterator_func)(CityRides *, void *))
{
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, rides->cityTable);
	gpointer value;
	CityRides cityRides;
	while (g_hash_table_iter_next(&iter, NULL, &value))
	{
		// value é gptrarray mas temos de passar como cityTable
		cityRides.array = (GPtrArray *)value;
		iterator_func(&cityRides, data);
	}
}

#define BSEARCH_START 0
#define BSEARCH_END 1

// coisa mais manhosa que a manhosidade
// adaptação do bsearch que pode ou não funcionar
// se der errado ao menos é rápido
int custom_bsearch(GPtrArray *array, char *date, int mode) {
	int lim = array->len - 1,
	base = 0,
	cmp = 0,
	index = 0, len = array->len;
	RidesStruct *ride;
	for (; lim != 0; lim >>= 1) {
		index = base + (lim >> 1);
		ride = g_ptr_array_index(array, index);
		cmp = compDates(date, ride->date);
		if (cmp == 0) {
			// return index;
			break;
		}
		if (cmp > 0) {
			base = index + 1; // + 1 ??? que coisa maquiavélica
			lim--;
		}
	}
	
	// bounds check????????
	// printf("initial cmp: %d\n", cmp);
	if (cmp > 0) {
		for (; cmp > 0; index ++) {
			ride = g_ptr_array_index(array, index);
			cmp = compDates(date, ride->date);
		}
		index--;
		if (mode == BSEARCH_END) index--;
	} else if (cmp < 0) {
		for (; cmp < 0; index --) {
			ride = g_ptr_array_index(array, index);
			cmp = compDates(date, ride->date);
		}
		index++;
		if (mode == BSEARCH_START) index++;
	} else { //???????????? parece funcionar
		if (mode == BSEARCH_START && index > 0) {
			do {
				index --;
				ride = g_ptr_array_index(array, index);
				cmp = compDates(date, ride->date);
			} while (index >= 0 && cmp == 0);
			index++;
		} else if (index < len - 1) {
			do {
				index++;
				ride = g_ptr_array_index(array, index);
				cmp = compDates(date, ride->date);
			} while (index < len && cmp == 0);
			index--;
		}
	}

	return index;
}

void searchCityRidesByDate(CityRides *rides, char *dateA, char *dateB, int *res_start, int *res_end) {
	GPtrArray *array = rides->array;
	int len = array->len;
	char *first, *last;
	RidesStruct *ride;
	ride = g_ptr_array_index(array, 0);
	first = ride->date;
	ride = g_ptr_array_index(array, len - 1);
	last = ride->date;

	// printf("-------------------------------\nRequested dates: %s %s\n", dateA, dateB);
	// printf("First: %s last: %s\n", first, last);

	//???????
	if (compDates(dateA, first) < 0) {
		if (compDates(dateB, last) > 0) {
			*res_start = 0;
			*res_end = len - 1;
		} else if (compDates(dateB, first)) {
			*res_start = -1;
			*res_end = -1;
		} else {
			*res_start = 0;
			*res_end = custom_bsearch(array, dateB, BSEARCH_END);
		}
	} else {
		if (compDates(dateA, last) > 0) {
			*res_start = -1;
			*res_end = -1;
		} else if (compDates(dateB, last) < 0) {
			*res_start = custom_bsearch(array, dateA, BSEARCH_START);
			*res_end = custom_bsearch(array, dateB, BSEARCH_END);
		} else {
			*res_start = custom_bsearch(array, dateA, BSEARCH_START);
			*res_end = len - 1;
		}
	}
	// printf("got indices: %d %d\n", *res_start, *res_end);
	// printf("these correspond to %s %s\n", ((RidesStruct *)g_ptr_array_index(array, *res_start))->date, ((RidesStruct *)g_ptr_array_index(array, *res_end))->date);
}

void dumpCityRidesDate (char * filename, CityRides * rides) {
	FILE *fp = fopen(filename, "w");
	int i;
	GPtrArray *array = rides->array;
	RidesStruct *ride;
	for (i = 0; i < (const int)array->len; i++) {
		ride = g_ptr_array_index(array, i);
		fprintf(fp, "%d %s\n", i, ride->date);
	}
	fclose(fp);
}


// devolve a struct(dados) associada à ride número i
RidesStruct *getRidePtrByID(RidesData *data, guint ID)
{
	ID -= 1; // para o primeiro passar a ser 0
	int i = ID / RIDES_ARR_SIZE;

	RidesStruct **primaryArray = data->ridesArray,
				*secondaryArray = primaryArray[i],
				*result = &(secondaryArray[ID - SIZE * i]);
	return result;
}

char *getRideDate(RidesStruct *ride)
{
	return strndup(ride->date, RIDE_STR_BUFF);
}

short int getRideDriver(RidesStruct *ride)
{
	return (ride->driver);
}

char *getRideUser(RidesStruct *ride)
{
	return strndup(ride->user, RIDE_STR_BUFF);
}

char *getRideCity(RidesStruct *ride)
{
	return strndup(ride->city, RIDE_STR_BUFF);
}

short int getRideDistance(RidesStruct *ride)
{
	return (ride->distance);
}

short int getRideScore_u(RidesStruct *ride)
{
	return (ride->score_u);
}

short int getRideScore_d(RidesStruct *ride)
{
	return (ride->score_d);
}

float getRideTip(RidesStruct *ride)
{
	return (ride->tip);
}

char *getRideComment(RidesStruct *ride)
{
	return strndup(ride->comment, RIDE_STR_BUFF);
}
