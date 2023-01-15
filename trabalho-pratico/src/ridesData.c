#include "ridesData.h"

#define RIDE_STR_BUFF 32
#define N_OF_FIELDS 10

#define RIDE_IS_VALID(ride) (ride->date != NULL)

struct RidesStruct
{
	int ID;
	char *date;
	int driver;
	char *user;
	char *city;
	short int distance,
		score_u,
		score_d;
	float tip;
	//char *comment;
};

typedef struct {
	GThread * thread;
	CityRides **cityRidesPtrArray;
	int len,
	n_of_drivers;
} ThreadStruct;

// cada key da GHashTable cityTable aponta para uma struct deste tipo
// apenas entram nas cityRides entradas válidas, já que não se faz get por ID
struct CityRides {
	GPtrArray * cityRidesArray;
	GPtrArray * driverSumArray;
}; 

struct ridesByDriver {
	GPtrArray * ridesArray;
};

//informações de todas a rides para um user
struct driverRatingInfo {
    void * ratingChart; // void * porque vai guardar um array de ratings de 1 a 5 e distância viajada, e mais tarde é convertido para um (double *) do valor médio desses ratings
    double tips; // inicialmente guarda número de tips, depois guarda o total_auferido
    char * mostRecRideDate; // ride mais decente
    short int  * ridesNumber; // guardar [número de rides, total viajado]
    int driverNumber; // talvez meter em int o valor, ocupa menos espaço com char?
};

//função que guarda arrays criados e acedidos recorrentemente, para reduzir tempo de processamento
typedef struct querySavedData {
	GPtrArray * driverRatingArray; // array de dados médios sobre cada driver, para o global das rides (query 2)
} querySavedData;

//mudar para g_array no futuro?? glib faz alloc dinamico?
typedef struct {
	int len;
	RidesStruct ridesArray[SIZE]; // já sabemos que vai levar malloc de SIZE e já, e assim usamos g_ptr_array
} SecondaryRidesArray;

struct RidesData {
	GPtrArray * ridesArray;
	GHashTable *cityTable;
	GPtrArray * driverInfoArray;
	querySavedData * savedData;
};

void freeCityRides(void *data);
gint compareRidesByDate(gconstpointer, gconstpointer);
void *buildStatisticsInCity(void *);
SecondaryRidesArray *getRides(FILE *, GHashTable *, parse_format *, int *);

GPtrArray * buildRidesbyDriverSorted (GPtrArray *);
GPtrArray * buildRidesByDriverInCity(GPtrArray * ridesInCity, int);
GPtrArray * buildRidesByDriverGlobal(GHashTable *, int);
GPtrArray * addDriverInfo(GPtrArray *,RidesStruct *);
driverRatingInfo * newDriverInfo (RidesStruct *);
driverRatingInfo *newDriverGlobalInfo(driverRatingInfo *);
driverRatingInfo * appendDriverInfo (driverRatingInfo *, RidesStruct *);
driverRatingInfo * appendDriverGlobalInfo(driverRatingInfo *, driverRatingInfo *);
GPtrArray * getPresentableValues (GPtrArray *);
driverRatingInfo * sumValues (driverRatingInfo *);
driverRatingInfo * newOpaqueDriverInfo (int);

void freeRidesPtrArray (void *);
void freeRidesRating(void *);

void *buildStatisticsInCity(void *data) {
	ThreadStruct *thread = (ThreadStruct *)data;
	int i;
	CityRides * cityData;
	GPtrArray * cityRidesArray;
	for (i = 0; i < thread->len; i++) {
		cityData = thread->cityRidesPtrArray[i];
		cityRidesArray = cityData->cityRidesArray;
		g_ptr_array_sort(cityRidesArray, compareRidesByDate); // a função buildRidesByDriverInCity precisa de datas ordendadas, aproveita-se e faz-se esse calculo antes
		cityData->driverSumArray = buildRidesByDriverInCity(cityRidesArray, thread->n_of_drivers);
	}
	// g_thread_exit(NULL);
	return NULL;
}

RidesData * getRidesData(FILE *ptr, int numberOfDrivers) {
	int i;

	//inicializar as estruturas de dados relacionadas com as rides
	GPtrArray * ridesArray = g_ptr_array_new_with_free_func(freeRidesPtrArray);
	SecondaryRidesArray * secondaryArray;

	GHashTable *cityTable = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, freeCityRides); // keys levam malloc do array normal, nao vou dar free aqui;
	
	parse_format format;

	parse_func_struct format_array[N_OF_FIELDS] = {
		{ p_getID, offsetof(RidesStruct, ID), 0, },
		{ p_getDate, offsetof(RidesStruct, date), 1, },
		{ p_getDriver, offsetof(RidesStruct, driver), 0, },
		{ p_getName, offsetof(RidesStruct, user), 1, },
		{ p_getCity, offsetof(RidesStruct, city), 1, },
		{ p_getDistance, offsetof(RidesStruct, distance), 0, },
		{ p_getScoreUser,offsetof(RidesStruct, score_u), 0, },
		{ p_getScoreDriver, offsetof(RidesStruct, score_d), 0, },
		{ p_getTip, offsetof(RidesStruct, tip), 0, },
		{ p_getComment, 0, 0 }, // comment nao é guardado mas temos de lhe dar skip
	};

	format.format_array = format_array;
	format.len = N_OF_FIELDS;

	int invalid = 0;
	while (fgetc(ptr) != '\n'); // avançar a primeira linha (tbm podia ser um seek hardcoded)

	secondaryArray = getRides(ptr, cityTable, &format, &invalid); // porque é que este loop ta feito assim????

	while (secondaryArray != NULL) {
		g_ptr_array_add(ridesArray, secondaryArray);
		secondaryArray = getRides(ptr, cityTable, &format, &invalid);
	}

	int num = (ridesArray->len - 1) * SIZE;
	secondaryArray = g_ptr_array_index(ridesArray, ridesArray->len - 1);
	num += secondaryArray->len;
	printf("Total number of rides: %d\nNumber of invalid rides: %d\n", num, invalid);

	//## MT para cálulco de estatística de dados para cada cidade
	// assumimos que o nº de cidades esta bem distribuido (na fase 1 pelo menos estava)
	
	guint num_cities = g_hash_table_size(cityTable);

	int num_threads = num_cities <= N_OF_THREADS ? num_cities : N_OF_THREADS;
	// cities_per_thread = (num_cities / num_threads) + 1;

	CityRides *cityRidesArray[num_cities]; // é um único array
	ThreadStruct thread_info[num_threads], *currentThread;

	// printf("%d citites, %d available cores, spawning %d extra threads\n",(int)num_cities, N_OF_THREADS, num_threads - 1);

	GHashTableIter iter;
	gpointer value;
	g_hash_table_iter_init (&iter, cityTable);
	gpointer temp;

	for (i = 0; i < num_threads; i++) {
		thread_info[i].len = 0;
		thread_info[i].n_of_drivers = numberOfDrivers;
	}

	// preencher array de CityRides * que as threads vão usar
	// i < ... é preciso???
	for (i = 0; g_hash_table_iter_next (&iter, &temp, &value) && i < (int)num_cities; i++) {
		cityRidesArray[i] = (CityRides *)value;
		currentThread = &thread_info[i % num_cities]; // quick mafs estao certas???
		// printf("city %d (%s) assigned to thread %d\n", i, (char *)temp, i % num_cities);
		if (currentThread->len == 0) {
			currentThread->cityRidesPtrArray = &cityRidesArray[i];
			// printf("This was the first city for this thread\n");
		}
		(currentThread->len)++;
	}

	for (i = 0; i < num_threads - 1; i++) {
		currentThread = &thread_info[i];
		currentThread->thread = g_thread_new(NULL, buildStatisticsInCity, currentThread);
	}
	// main thread é a [num_threads - 1]
	currentThread = &thread_info[i];
	buildStatisticsInCity(currentThread);

	// ultima thread é a main, por isso nao precisa de join
	for (i = 0; i < (int)num_cities - 1; i++) {
		g_thread_join(thread_info[i].thread);
	}
	
	GPtrArray * driverInfoArray = buildRidesByDriverGlobal(cityTable,numberOfDrivers);

	RidesData *data = malloc(sizeof(RidesData));
	data->ridesArray = ridesArray; // array com input do ficheiro
	data->cityTable = cityTable; // hash table que guarda structs com: array com rides ordenandas para uma cidade key, e array com info resumida de drivers nessa cidade;
	data->driverInfoArray = driverInfoArray; // array com informação de drivers resumida global
	data->savedData = malloc(sizeof(querySavedData)); // estrutura em que se guarda novos array criados nas queries
	data->savedData->driverRatingArray = buildRidesbyDriverSorted(driverInfoArray);
	//dumpDriverInfoArray("query_2-ouputs",data->savedData->driverRatingArray,NULL,1);
	return data;
}

SecondaryRidesArray *getRides(FILE *ptr, GHashTable *cityTable, parse_format *format, int *invalid) {
	
	int i, res;
	char *city;

	SecondaryRidesArray *secondaryArrayStruct = malloc(sizeof(SecondaryRidesArray));
	RidesStruct * ridesStructArray = secondaryArrayStruct->ridesArray, *temp;
	CityRides * cityRides;

	for (i = 0; i < SIZE; i++) {

		if ((res = parse_with_format(ptr, (void *)&ridesStructArray[i], format)) == 1) {
			city = ridesStructArray[i].city;
			temp = &(ridesStructArray[i]);

			// check if city is not already in hash table
			if ((cityRides = g_hash_table_lookup(cityTable, city)) == NULL)
			{
				// if not, insert
				cityRides = malloc(sizeof(CityRides));
				cityRides->cityRidesArray = g_ptr_array_sized_new(200000);
				cityRides->driverSumArray = NULL;
				g_ptr_array_add(cityRides->cityRidesArray, temp);
				g_hash_table_insert(cityTable, city, cityRides);
			}
			else
			{
				// if yes, append to all the other data
				g_ptr_array_add(cityRides->cityRidesArray, temp);
			}
		} else if (res == -1) {// se chegarmos a EOF
				if (i == 0) {
					free(secondaryArrayStruct);
					secondaryArrayStruct = NULL;
				}
				break;
		} else {
			(*invalid)++;
		}
	}

	if (secondaryArrayStruct != NULL) secondaryArrayStruct->len = i;

	return secondaryArrayStruct;
}

GPtrArray *addDriverInfo(GPtrArray *driverInfoArray, RidesStruct *currentRide) {
	driverRatingInfo *newStruct = NULL, // pointer para struct que vai ser guardada em cada pos do array
		*currentArrayStruct = NULL;
	int driverNumber = currentRide->driver;
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

driverRatingInfo *appendDriverInfo(driverRatingInfo *currentArrayStruct, RidesStruct *currentRide) {
	currentArrayStruct->tips += currentRide->tip;

	unsigned int *ratings = (unsigned int *)currentArrayStruct->ratingChart;
	ratings[(currentRide->score_d) - 1]++; // dependendo da avaliação na ride atual, no array ratingChart , incrementa em 1 o número da avalições de valor 1,2,3,4 ou 5.

	short int *ridesTracker = (short int *)currentArrayStruct->ridesNumber;
	ridesTracker[1] += currentRide->distance;

	//como assumo que o array de rides recebido já está sorted, não é preciso comparar se novas inserções têm datas mais recentes, pois serão sempre
	free(currentArrayStruct->mostRecRideDate);
	currentArrayStruct->mostRecRideDate = strdup(currentRide->date);

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
	newStruct->tips = 0;

	// if(fullyOpaque == TRUE) { 
	// 	newStruct->mostRecRideDate = strdup("00/00/0000"); 
	// 	newStruct->ridesNumber = calloc(2,sizeof(short int));} else { 

	// para poupar memória não se preenche estes campos
	newStruct->mostRecRideDate = NULL;
	newStruct->ridesNumber = NULL;
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
	// RidesStruct **ridesData = dataStruct->ridesArray;

	// free de rides por cidade
	GHashTable *table = dataStruct->cityTable;
	g_hash_table_destroy(table);
	// free do array com info resumida de drivers
	GPtrArray *driverInfoArray = dataStruct->driverInfoArray;
	g_ptr_array_free(driverInfoArray, TRUE);
	// free de novos arrays criados nas queries
	querySavedData * savedData = dataStruct->savedData;
	g_ptr_array_free(savedData->driverRatingArray,TRUE); 
	free(data->savedData);
	//free das rides
	GPtrArray * ridesArray = dataStruct->ridesArray;
	g_ptr_array_free(ridesArray, TRUE);

	free(dataStruct);
}

void freeRidesPtrArray (void * data) {

	SecondaryRidesArray * secondaryArrayStruct = (SecondaryRidesArray *) data;
	RidesStruct * ridesArray = (RidesStruct *) secondaryArrayStruct->ridesArray;

	int i, secondaryArraySize = secondaryArrayStruct->len;
	RidesStruct *currentRideStruct;
	for (i=0; i<secondaryArraySize; i++) {
		currentRideStruct = &ridesArray[i];
		if (RIDE_IS_VALID(currentRideStruct)) {
			free(currentRideStruct->date);
			free(currentRideStruct->user);
			free(currentRideStruct->city);
		}
	//free(currentRideStruct->comment);
	}
	free(secondaryArrayStruct);
}

void freeRidesRating(void *drivesRating) {
	driverRatingInfo *currentArrayStruct = (driverRatingInfo *)drivesRating;
	free(currentArrayStruct->ratingChart);
	free(currentArrayStruct->mostRecRideDate);
	free(currentArrayStruct->ridesNumber);
	free(currentArrayStruct);
}

void freeCityRides(void *data) {
	CityRides * cityInfo = (CityRides *)data;
	g_ptr_array_free(cityInfo->cityRidesArray, TRUE);
	g_ptr_array_free(cityInfo->driverSumArray, TRUE);
	free(cityInfo);
}

//funções relativas a rides ordenadas por driver
const ridesByDriver * getRidesByDriver(const RidesData * ridesData) {// responsabilidade da caller function dar free
	ridesByDriver * driverInfoArray = malloc(sizeof(ridesByDriver));
	driverInfoArray->ridesArray = ridesData->driverInfoArray;
	return driverInfoArray;
}

const ridesByDriver * getRidesByDriverSorted(const RidesData * ridesData) {
	ridesByDriver * driverRatingArray = malloc(sizeof(ridesByDriver));
	driverRatingArray->ridesArray = ridesData->savedData->driverRatingArray;
	return driverRatingArray;
}

//wrapper para o array com resumo da info de cada driver numa dada cidade
const ridesByDriver * getRidesByDriverInCity(const CityRides * cityRides) {
	ridesByDriver * driverInfoArrayWrapper = malloc(sizeof(ridesByDriver));
	driverInfoArrayWrapper->ridesArray = cityRides->driverSumArray; // supõe que a key existe sempre, se não for o caso passar a usar g_hash_table_lookup_extended
	return driverInfoArrayWrapper;
}

const driverRatingInfo *getDriverInfo(const ridesByDriver *ridesByDriver, guint id)
{
	GPtrArray *array = ridesByDriver->ridesArray;
	// if (id > array->len) return NULL;
	driverRatingInfo *currentArrayStruct = (driverRatingInfo *)g_ptr_array_index(array, id - 1);
	return (currentArrayStruct);
}

short int getridesByDriverArraySize(const ridesByDriver *driverInfoArray)
{
	return (driverInfoArray->ridesArray->len);
}

gint sort_byRatings_2 (gconstpointer a, gconstpointer b) {
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

gint sort_byRatings_7 (gconstpointer a, gconstpointer b) {
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
		result = (int)(driver1->driverNumber - driver2->driverNumber); // tem de se fazer a comparação em ordem decrescente, daí 1 - 2 (n sei bem porquê, mas funciona?);
	}
    return result;
}

GPtrArray * buildRidesbyDriverSorted (GPtrArray * driverInfoArray) {
	GPtrArray * driverRatingArray = g_ptr_array_copy (driverInfoArray,NULL,NULL);
	g_ptr_array_set_free_func(driverRatingArray,NULL); // o novo array copia a função de free do anterior, mas como é um pointer array oco, não dá free dos elementos para que aponta
	g_ptr_array_sort(driverRatingArray, sort_byRatings_2);
	return driverRatingArray;
}

//constroi o resumo da info de um driver para um dado conjunto de rides (utilizado em rides de cada cidade, daí o nome de momento)
GPtrArray * buildRidesByDriverInCity(GPtrArray * ridesInCity, int numberOfDrivers) {
	gint i, numberOfRides = ridesInCity->len; // já é dinâmico
	RidesStruct * currentArrayStruct;
	GPtrArray * driverInfoArray = g_ptr_array_new_full(numberOfDrivers, freeRidesRating);
	g_ptr_array_set_size(driverInfoArray,numberOfDrivers); // o tamanho do array tem de ser definido, apesar de ja ter sido alocado o espaço para o tamanho necessário; o array tem de ser inicializado a NULL para todos os pointers, feito por g_ptr_array_set_size

	for (i=0;i<numberOfRides;i++) {
		currentArrayStruct = g_ptr_array_index(ridesInCity, i);
		driverInfoArray = addDriverInfo(driverInfoArray, currentArrayStruct);
	}
	//driverInfoArray = getPresentableValues(driverInfoArray);
	//g_ptr_array_sort(driverInfoArray,sort_byRatings_7);
	return driverInfoArray;
}

driverRatingInfo *newDriverGlobalInfo(driverRatingInfo *currentDriverInfo) {
	driverRatingInfo * newStruct = malloc(sizeof(driverRatingInfo));
	newStruct->driverNumber = currentDriverInfo->driverNumber;
	newStruct->mostRecRideDate = strdup(currentDriverInfo->mostRecRideDate);
	newStruct->tips = currentDriverInfo->tips;
	newStruct->ridesNumber = calloc(2, sizeof(short int));
	memcpy(newStruct->ridesNumber,currentDriverInfo->ridesNumber,sizeof(short int) * 2);
	newStruct->ratingChart = calloc(5, sizeof(unsigned int));
	memcpy(newStruct->ratingChart,currentDriverInfo->ratingChart,sizeof(unsigned int) * 5);
	return newStruct;		
}

driverRatingInfo * appendDriverGlobalInfo(driverRatingInfo *currentGlobalArrayStruct, driverRatingInfo * currentCityArrayStruct) {
	short int i;
	unsigned int * ratingsTo, * ratingsFrom;
	char * CityMostRecRideDate = currentCityArrayStruct->mostRecRideDate; // teste de um driver n ter informação na cidade é data == NULL
	char * GlobalMostRecRideDate = currentGlobalArrayStruct->mostRecRideDate;
	if (compDates(CityMostRecRideDate, GlobalMostRecRideDate) >= 0) {
		free(GlobalMostRecRideDate);
		currentGlobalArrayStruct->mostRecRideDate = strdup(CityMostRecRideDate);
	}
	ratingsTo = (unsigned int *)currentGlobalArrayStruct->ratingChart;
	ratingsFrom = (unsigned int *)currentCityArrayStruct->ratingChart;
	for (i=0;i<5;i++) {
		ratingsTo[i] += ratingsFrom[i];
	}
	currentGlobalArrayStruct->tips += currentCityArrayStruct->tips;
	currentGlobalArrayStruct->ridesNumber[0] += currentCityArrayStruct->ridesNumber[0];
	currentGlobalArrayStruct->ridesNumber[1] += currentCityArrayStruct->ridesNumber[1];
	return currentGlobalArrayStruct;
}

//pega na info resumidos de cada driver para uma cidade e adiciona-a à info de cada driver global
GPtrArray * buildRidesByDriverGlobal(GHashTable * cityTable, int numberOfDrivers) {
	int i,j;
	driverRatingInfo * newStruct, * currentCityArrayStruct, * currentGlobalArrayStruct;

	//criar structs com valores a 0 em cada posição, para acumular valores nessa posição depois
	// !! outra forma mais eficiente sem meter NULL com set_size?
	GPtrArray * driverInfoGlobalArray = g_ptr_array_new_full(numberOfDrivers, freeRidesRating);
	g_ptr_array_set_size(driverInfoGlobalArray,numberOfDrivers); // o tamanho do array tem de ser definido, apesar de ja ter sido alocado o espaço para o tamanho necessário; o array tem de ser inicializado a NULL para todos os pointers, feito por g_ptr_array_set_size

	GHashTableIter iter;
	gpointer value;
	guint num_keys = g_hash_table_size(cityTable);
	g_hash_table_iter_init (&iter, cityTable);
	for (i = 0; g_hash_table_iter_next (&iter, NULL, &value) && i <= (int)num_keys; i++) {	
		CityRides * cityData = (CityRides * ) value;
		GPtrArray * driverInfoInCityArray = cityData->driverSumArray;
		for (j=0;j<numberOfDrivers;j++) {
				currentGlobalArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverInfoGlobalArray, j);
				currentCityArrayStruct = (driverRatingInfo *) g_ptr_array_index(driverInfoInCityArray, j);
			if (currentCityArrayStruct != NULL) {
				if (currentGlobalArrayStruct == NULL) { // seria mais simples correr um loop inicial em que inicia com valores da cityatual, ou valores ocos se NULL a cityatual?
					newStruct = newDriverGlobalInfo(currentCityArrayStruct);
					g_ptr_array_index(driverInfoGlobalArray, j) = newStruct;
				} else{
					currentGlobalArrayStruct = appendDriverGlobalInfo(currentGlobalArrayStruct,currentCityArrayStruct);
				}
			}
		}
		//cálculo das médias e ordenação da info sobre drivers para cada cidade individualmente (usado na query 7)
		cityData->driverSumArray=getPresentableValues(driverInfoInCityArray); // meter com MT também?? 
		g_ptr_array_sort(cityData->driverSumArray,sort_byRatings_7); // meter com MT também??
	}
	driverInfoGlobalArray = getPresentableValues(driverInfoGlobalArray);
	return driverInfoGlobalArray;
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

int getDriverNumber(const driverRatingInfo *currentArrayStruct)
{
	return currentArrayStruct->driverNumber;
}

// funções relativas a rides ordenadas por cidade
CityRides *getRidesByCity(RidesData *data, char *city) // responsabilidade da caller function dar free
{
	return g_hash_table_lookup(data->cityTable, city);
}

guint getNumberOfCityRides(CityRides *rides)
{
	return rides->cityRidesArray->len;
}

RidesStruct *getCityRidesByIndex(CityRides *rides, guint ID)
{
	return (RidesStruct *)g_ptr_array_index(rides->cityRidesArray, (int)ID);
}

void iterateOverCities(RidesData *rides, void *data, void (*iterator_func)(CityRides *, void *))
{
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, rides->cityTable);
	gpointer value;
	while (g_hash_table_iter_next(&iter, NULL, &value))
	{
		// value é gptrarray mas temos de passar como cityTable
		iterator_func(value, data);
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

void searchCityRidesByDate(CityRides * cityRides, char *dateA, char *dateB, int *res_start, int *res_end) {
	GPtrArray *array = cityRides->cityRidesArray;
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

//função de debug, pode receber a hash table inteira, a struct inteira (com as rides e info resumida), ou só o array das rides
// escolher um formato de input e meter os restantes a NULL
void dumpCityRides (char * filename, GHashTable * cityTable, CityRides * rides, GPtrArray * ridesArray) {
	FILE * fp;
	int i,j;
	GPtrArray *array;
	RidesStruct *ride;
	if (cityTable != NULL) {
		guint num_keys = g_hash_table_size(cityTable);
		GHashTableIter iter;
		gpointer value;
		CityRides * cityData;
		char * path = malloc(sizeof(char) *50);
		g_hash_table_iter_init (&iter, cityTable);
		for (i = 0; g_hash_table_iter_next (&iter, NULL, &value) && i <= (int)num_keys; i++) {
			cityData = (CityRides *)value;
			sprintf(path,"%s-city%d",filename,i);
			fp = fopen(path, "w");
			array = cityData->cityRidesArray;
			for (j = 0; j < (const int)array->len; j++) {
				ride = g_ptr_array_index(array, j);
				//fprintf(fp,"%s",ride->date);
				fprintf(fp, "%s iter%d | date:%s, driver: %d, distance: %d\n", ride->city,j, ride->date,ride->driver,ride->distance);
			}
			fclose(fp);
		}
		free(path);
	} else {
		fp = fopen(filename, "w");
		array = ridesArray;
		if (rides != NULL) array = rides->cityRidesArray;
		for (i = 0; i < (const int)array->len; i++) {
			ride = g_ptr_array_index(array, i);
			//fprintf(fp,"%s",ride->date);
			fprintf(fp, "%s iter%d | date:%s, driver: %d, distance: %d\n",ride->city,i,ride->date,ride->driver,ride->distance);
		}
	fclose(fp);
	}
}

//dump driverInfoArray: dataState == 0 -> before getPresentablevalues, dataState == 1 -> after getPresentablevalues!!
void dumpDriverInfoArray (char * filename, GPtrArray * driverInfo, char * addToFilename, int dataState) {
	if(addToFilename != NULL) filename = strcat(filename,addToFilename);
	FILE *fp = fopen(filename, "w");
	int i, numberOfDrivers = driverInfo->len;
	driverRatingInfo * currentDriver;
	for (i = 0; i < numberOfDrivers; i++) {
		currentDriver = g_ptr_array_index(driverInfo, i);
		if (currentDriver == NULL) fprintf(fp,"driverNumber:%d | NO INFO!\n",i+1);
		else {
			if (!dataState) {
			unsigned int * ratings = (unsigned int *)currentDriver->ratingChart;
			//NumberOfRides não foi calculado ainda, só depois de getPresentableValues, por isso não é incluído no print
			fprintf(fp, "driverNumber:%d, mostRecdate: %s, avgRatings: [%d,%d,%d,%d,%d], totalTravalled:%d\n", currentDriver->driverNumber,currentDriver->mostRecRideDate,ratings[0],ratings[1],ratings[2],ratings[3],ratings[4],currentDriver->ridesNumber[1]);
			}
			else {
				fprintf(fp,"driverNumber:%d, mostRecdate:%s, avgRating: %f\n",currentDriver->driverNumber,currentDriver->mostRecRideDate, *(double *)currentDriver->ratingChart);
			}
		}
	}
	fclose(fp);
}

// devolve a struct(dados) associada à ride número i
RidesStruct * getRidePtrByID(RidesData *data, guint ID)
{
	ID -= 1; // para a primeira ride passar a ser 0
	guint i = ID / SIZE;
	GPtrArray *array = data->ridesArray;
	if (i > array->len) return NULL;

	SecondaryRidesArray *secondaryArray = g_ptr_array_index(array, i);

	RidesStruct * result = &(secondaryArray->ridesArray[ID - SIZE * i]);
	if (result == NULL || !RIDE_IS_VALID(result)) return NULL;
 
	return result;
}

int getRideID(const RidesStruct * ride) {
	return ride->ID;
}

char *getRideDate(const RidesStruct *ride){
	return strndup(ride->date, RIDE_STR_BUFF);
}

int getRideDriver(RidesStruct *ride) {
	return (ride->driver);
}

char *getRideUser(RidesStruct *ride) {
	return strndup(ride->user, RIDE_STR_BUFF);
}

char *getRideCity(const RidesStruct *ride) {
	return strndup(ride->city, RIDE_STR_BUFF);
}

short int getRideDistance(const RidesStruct *ride) {
	return (ride->distance);
}

short int getRideScore_u(RidesStruct *ride) {
	return (ride->score_u);
}

short int getRideScore_d(RidesStruct *ride) {
	return (ride->score_d);
}

float getRideTip(const RidesStruct *ride) {
	return (ride->tip);
}

// char *getRideComment(RidesStruct *ride)
// {
// 	return strndup(ride->comment, RIDE_STR_BUFF);
// }

int rideIsValid(RidesStruct *ride) {
	return (ride != NULL && RIDE_IS_VALID(ride));
}

int getNumberOfRides(RidesData * data) {
	GPtrArray * array = data->ridesArray;
	int num = (array->len - 1) * SIZE;
	SecondaryRidesArray * secondaryArray = g_ptr_array_index(array, array->len - 1);
	num += secondaryArray->len;
	return num;
}
