#include "ridesData.h"
#include <stdint.h>

#define RIDE_STR_BUFF 32
#define N_OF_FIELDS 10
#define SIZE (1 << 17)
#define RIDES_PER_CITY 200000
#define RIDE_IS_VALID(ride) (ride->user != NULL)

struct RidesStruct
{
	int ID;
	Date date;
	int driver;
	char *user;
	char *city;
	short int distance,
		score_u,
		score_d;
	float tip;
	//char *comment;
};

typedef union { // guarda um array de ratings de 1 a 5, e mais tarde o valor médio desses ratings
	unsigned int chart[5];
	double average;
} ratingsUnion;

//informações de todas a rides para um user
struct driverRatingInfoGlobal {
	ratingsUnion ratings;
	short int ridesTracker[2]; // guardar [número de rides, total viajado]
    double tips; // inicialmente guarda número de tips, depois guarda o total_auferido
    Date mostRecRideDate; // ride mais recente
    int driverNumber; // tem de ser int, em large-dataset temos drivers até 100.000
};

struct driverRatingInfoCity {
	ratingsUnion ratings;
	int driverNumber;
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
	driverRatingInfoCity ** driverSumArray; // array com informação resumida de cada driver, para uma dada cidade
	int distance[3], total[3], 
		numberOfDrivers; // necessário para o free do array driverSumArray
}; 

struct ridesByDriverGlobal {
	driverRatingInfoGlobal ** ridesArray;
	int len;
};

struct ridesByDriverCity {
	driverRatingInfoCity ** ridesArray;
	int len;
};

//mudar para g_array no futuro?? glib faz alloc dinamico?
typedef struct {
	int len;
	RidesStruct ridesArray[SIZE]; // já sabemos que vai levar malloc de SIZE e já, e assim usamos g_ptr_array
} SecondaryRidesArray;

struct RidesData {
	GPtrArray * ridesArray;
	GHashTable *cityTable;
	driverRatingInfoGlobal ** driverInfoArray; // info de drivers ordenada por ID
	driverRatingInfoGlobal ** driverRatingArray; // info de drivers ordenada por parâmetros da Q2
	int numberOfDrivers; // para saber o tamanho dos arrays no freeRidesData
};

void freeCityRides(void *data);
gint compareRidesByDate(gconstpointer, gconstpointer);
void *buildStatisticsInCity(void *);
SecondaryRidesArray *getRides(FILE *ptr, GHashTable *cityTable, const parse_format *format, int *invalid, const UserData *userdata, const DriverData *driverdata, driverRatingInfoGlobal **, int numberOfDrivers, int *bp, int *sp, char *buffer);

driverRatingInfoGlobal * * buildRidesbyDriverSorted (driverRatingInfoGlobal **, int);
driverRatingInfoCity * * buildRidesByDriverCity(GPtrArray *, int);
driverRatingInfoGlobal * * buildRidesByDriverGlobal(GHashTable *, driverRatingInfoGlobal **, int);

void addDriverInfoCity(driverRatingInfoCity **,RidesStruct *);
void addDriverInfoGlobal(driverRatingInfoGlobal **,RidesStruct *);
driverRatingInfoCity * newDriverInfoCity (RidesStruct *);
driverRatingInfoGlobal * newDriverInfoGlobal(RidesStruct *);
void appendDriverInfoCity (driverRatingInfoCity *, RidesStruct *);
void appendDriverInfoGlobal(driverRatingInfoGlobal *,RidesStruct *);
void getPresentableValues (driverRatingInfoGlobal **, int);
void sumValuesCity (driverRatingInfoCity *);
void sumValuesGlobal (driverRatingInfoGlobal *);
driverRatingInfoGlobal * newOpaqueDriverInfoGlobal (int);

void freeRidesPtrArray (void *);
void freeRidesRatingGlobal(driverRatingInfoGlobal **, int);
void freeRidesRatingCity(driverRatingInfoCity **, int);

//construir array de cada cidade, com multithreading
void *buildStatisticsInCity(void *data) {
	ThreadStruct *thread = (ThreadStruct *)data;
	CityRides * cityData;
	GPtrArray * cityRidesArray;
	int i;
	for (i = 0; i < thread->len; i++) {
		cityData = thread->cityRidesPtrArray[i];
		cityRidesArray = cityData->cityRidesArray;
		// testei usar qsort() e foi igual
		g_ptr_array_sort(cityRidesArray, compareRidesByDate); // a função buildRidesByDriverInCity precisa de datas ordendadas, aproveita-se e faz-se esse calculo antes
		cityData->driverSumArray = buildRidesByDriverCity(cityRidesArray, thread->n_of_drivers);
	}
	// g_thread_exit(NULL);
	return NULL;
}

void multiThreadedCityRides(GHashTable *cityTable, const DriverData *driverdata, int numberOfDrivers) {
	guint num_cities = g_hash_table_size(cityTable);

	GHashTableIter iter;
	gpointer value;
	g_hash_table_iter_init (&iter, cityTable);
	gpointer temp;
	int i;
	CityRides *cityRidesArray[num_cities]; // é um único array

	if (N_OF_THREADS == 1) { // modo sequencial, nao da spawn de nenhuma thread
		// while (g_hash_table_iter_next (&iter, &temp, &value)) {
		// 	buildStatisticsInCitySequencial((CityRides *)value, numberOfDrivers);
		// }

	} else {
		int num_threads = num_cities <= N_OF_THREADS ? num_cities : N_OF_THREADS;
		// cities_per_thread = (num_cities / num_threads) + 1;
		
		ThreadStruct thread_info[num_threads], *currentThread;

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
	}

}

RidesData * getRidesData(FILE *ptr, const UserData *userdata, const DriverData *driverdata, char *buffer) {

	int numberOfDrivers = getNumberOfDrivers(driverdata); // número de drivers, necessário para construir arrays com info de cada driver (Q1,Q2,Q7)

	//inicializar as estruturas de dados relacionadas com as rides lidas do ficheiro diretamente
	GPtrArray * ridesArray = g_ptr_array_new_with_free_func(freeRidesPtrArray);
	SecondaryRidesArray * secondaryArray;

	//inicializa uma hash table
	GHashTable *cityTable = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, freeCityRides); // keys levam malloc do array normal, nao vou dar free aqui;
	
	//inicializa um array com info de drivers global
	driverRatingInfoGlobal ** driverInfoGlobalArray = calloc(numberOfDrivers,sizeof(driverRatingInfoGlobal *));

	parse_format format;

	parse_func_struct format_array[N_OF_FIELDS] = {
		{ p_getID, offsetof(RidesStruct, ID), 0, },
		{ p_getDate, offsetof(RidesStruct, date), 0, },
		{ p_getID, offsetof(RidesStruct, driver), 0, },
		{ p_getString, offsetof(RidesStruct, user), 1, },
		{ p_getString, offsetof(RidesStruct, city), 1, },
		{ p_getShortPositiveInt, offsetof(RidesStruct, distance), 0, },
		{ p_getShortPositiveInt,offsetof(RidesStruct, score_u), 0, },
		{ p_getShortPositiveInt, offsetof(RidesStruct, score_d), 0, },
		{ p_getTip, offsetof(RidesStruct, tip), 0, },
		{ p_checkEmptyNewline, 0, 0 }, // comment nao é guardado mas temos de lhe dar skip
	};

	format.format_array = format_array;
	format.len = N_OF_FIELDS;

	int invalid = 0;
	while (fgetc(ptr) != '\n'); // avançar a primeira linha (tbm podia ser um seek hardcoded)

	int bp = 0, sp = 0;

	secondaryArray = getRides(ptr, cityTable, &format, &invalid, userdata, driverdata, driverInfoGlobalArray, numberOfDrivers, &bp, &sp, buffer);

	while (secondaryArray != NULL) {
		g_ptr_array_add(ridesArray, secondaryArray);
		secondaryArray = getRides(ptr, cityTable, &format, &invalid, userdata, driverdata, driverInfoGlobalArray, numberOfDrivers, &bp, &sp, buffer);
	}

	int num = (ridesArray->len - 1) * SIZE;
	secondaryArray = g_ptr_array_index(ridesArray, ridesArray->len - 1);
	num += secondaryArray->len;
	printf("Total number of rides: %d\nNumber of invalid rides: %d\n", num, invalid);

	//## MT para cálulco de estatística de dados para cada cidade
	// assumimos que o nº de cidades esta bem distribuido (na fase 1 pelo menos estava)
	multiThreadedCityRides(cityTable, driverdata, numberOfDrivers);
	
	buildRidesByDriverGlobal(cityTable,driverInfoGlobalArray, numberOfDrivers);

	RidesData *data = malloc(sizeof(RidesData));
	data->numberOfDrivers = numberOfDrivers; // usado para o freeRidesArray
	data->ridesArray = ridesArray; // array com input do ficheiro
	data->cityTable = cityTable; // hash table que guarda structs com: array com rides ordenandas para uma cidade key, e array com info resumida de drivers nessa cidade;
	data->driverInfoArray = driverInfoGlobalArray; // array com informação de drivers resumida global
	data->driverRatingArray = buildRidesbyDriverSorted(driverInfoGlobalArray,numberOfDrivers);
	sort_userarray(userdata);
	//dumpDriverInfoArray("query_2-ouputs",data->savedData->driverRatingArray,NULL,1);
	return data;
}

SecondaryRidesArray *getRides(FILE *ptr, GHashTable *cityTable, const parse_format *format, int *invalid, const UserData *userdata, const DriverData *driverdata, driverRatingInfoGlobal ** driverArray, int numberOfDrivers, int *bp, int *sp, char *buffer) {
	
	int i, res;
	char *city;

	SecondaryRidesArray *secondaryArrayStruct = malloc(sizeof(SecondaryRidesArray));
	RidesStruct * ridesStructArray = secondaryArrayStruct->ridesArray, *temp;
	CityRides * cityRides;
	unsigned char carClass;

	for (i = 0; i < SIZE; i++) {

		if ((res = parse_with_format(ptr, (void *)&ridesStructArray[i], format, bp, sp, buffer)) == 1) {
			city = ridesStructArray[i].city;
			temp = &(ridesStructArray[i]);

			add_user_info(userdata, driverdata, temp->user, temp->driver, temp->distance, temp->score_u, temp->tip, temp->date);

			// check if city is not already in hash table
			if ((cityRides = g_hash_table_lookup(cityTable, city)) == NULL)
			{
				// if not, insert
				cityRides = malloc(sizeof(CityRides));
				cityRides->cityRidesArray = g_ptr_array_sized_new(RIDES_PER_CITY);
				cityRides->driverSumArray = calloc(numberOfDrivers,sizeof(driverRatingInfoCity *)); // array com info de drivers sobre a cidade específica; assume-se que NULL corresponde a tudo 0s!!!
				cityRides->numberOfDrivers = numberOfDrivers;
				addDriverInfoCity(cityRides->driverSumArray, temp); // adiciona a informação (numa cidade) do driver numa ride
				addDriverInfoGlobal(driverArray,temp); // adiciona informação (global) do driver nuam ride
				g_ptr_array_add(cityRides->cityRidesArray, temp); // adiciona a nova ride ao cityRidesarray (pointer array com rides de uma cidade)
				g_hash_table_insert(cityTable, city, cityRides); // insere a nova cityRide (estrutura de uma cidade) na hash table (com pointers para cada estrutura de uma cidade)

				memset(cityRides->distance, 0, sizeof(int) * 3);
				memset(cityRides->total, 0, sizeof(int) * 3);
			}
			else
			{
				// if yes, append to all the other data
				addDriverInfoCity(cityRides->driverSumArray, temp); // adiciona a informação (numa cidade) do driver numa ride
				addDriverInfoGlobal(driverArray,temp); // adiciona informação (global) do driver nuam ride
				g_ptr_array_add(cityRides->cityRidesArray, temp);
				carClass = getDriverCar(getDriverPtrByID(driverdata, temp->driver));
				cityRides->total[carClass] ++;
				cityRides->distance[carClass] += temp->distance;
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

//#define addDriverInfo (void * array, char type, currentRide) (type == 0 ? addDriverInfoCity(array,currentRide) : addDriverInfoGlobal(array,currentRide);

//adiciona driverRatingInfo de uma ride, numa cidade (Q7)
void addDriverInfoCity(driverRatingInfoCity ** driverInfoArray, RidesStruct *currentRide) {
	driverRatingInfoCity *currentArrayStruct = NULL;
	int driverNumber = currentRide->driver; // pode passar para bitshift na union no formato double depois??
	currentArrayStruct = driverInfoArray [driverNumber - 1];
	if (currentArrayStruct == NULL)
	{ // verifica se no local atual ainda n existe informação de um driver, e se este tem o estado ativo
		driverInfoArray[driverNumber-1] = newDriverInfoCity(currentRide);
	}
	else
	{ // se já existir informação de um driver (de rides prévias);
		appendDriverInfoCity(currentArrayStruct, currentRide);
	}
}

void addDriverInfoGlobal(driverRatingInfoGlobal ** driverInfoArray, RidesStruct *currentRide) {
	driverRatingInfoGlobal *currentArrayStruct = NULL;
	int driverNumber = currentRide->driver;
	currentArrayStruct = driverInfoArray [driverNumber - 1];
	if (currentArrayStruct == NULL)
	{ // verifica se no local atual ainda n existe informação de um driver, e se este tem o estado ativo
		driverInfoArray[driverNumber-1] = newDriverInfoGlobal(currentRide);
	}
	else
	{ // se já existir informação de um driver (de rides prévias);
		appendDriverInfoGlobal(currentArrayStruct, currentRide);
	}
}

// criar nova driverRatingInfo struct com informação de uma ride (primeira vez que aparece nas rides), numa cidade (Q7)
driverRatingInfoCity * newDriverInfoCity(RidesStruct *currentRide)
{
	driverRatingInfoCity *new = malloc(sizeof(driverRatingInfoCity));
	memset(new->ratings.chart,0,sizeof(unsigned int) * 5); // meter a zero as avaliações
	new->ratings.chart[(currentRide->score_d) - 1]++;
	new->driverNumber = currentRide->driver; // pode passar para o double na union com bitshift depois !
    return new;
}

// criar nova driverRatingInfo struct com informação de uma ride(primeira vez que aparece nas rides), global (Q1,Q2)
driverRatingInfoGlobal * newDriverInfoGlobal(RidesStruct * currentRide) {
	driverRatingInfoGlobal *new = malloc(sizeof(driverRatingInfoGlobal));
	//memset(new->ratings.chart,0,sizeof(unsigned int) * 5); // meter a zero as avaliações 
	//new->ratings.chart[(currentRide->score_d) - 1]++; // estes cálculos são feitos em cada cidade, no final pega-se no total
	new->ridesTracker[1] = currentRide->distance;
	new->tips = currentRide->tip;
	new->mostRecRideDate = currentRide->date;
	new->driverNumber = currentRide->driver;
    return new;
}

//adiciona driverRatingInfo sobre avaliação da ride atual, numa cidade (Q7)
inline void appendDriverInfoCity(driverRatingInfoCity * currentArrayStruct, RidesStruct *currentRide) {
	currentArrayStruct->ratings.chart[(currentRide->score_d) - 1]++; // dependendo da avaliação na ride atual, no array chart , incrementa em 1 o número da avalições de valor 1,2,3,4 ou 5.
	//comparar se novas inserções têm datas mais recentes
}

//adiciona driverRatingInfo sobre tips, distância percorrida e data mais recente, no global (Q1,Q2)
void appendDriverInfoGlobal(driverRatingInfoGlobal * currentArrayStruct, RidesStruct *currentRide) {
	currentArrayStruct->tips += currentRide->tip;
	// currentArrayStruct->ratings.chart[(currentRide->score_d) - 1]++; // cada cidade calcula isto, na construção final deste array apenas soma-se os valores de cada cidade, em vez de calcular aqui
	currentArrayStruct->ridesTracker[1] += currentRide->distance;
	//comparar se novas inserções têm datas mais recentes
	if (compDates(currentArrayStruct->mostRecRideDate,currentRide->date) < 0)
		currentArrayStruct->mostRecRideDate = currentRide->date;
}

void getPresentableValuesGlobal(driverRatingInfoGlobal ** driverInfoArray, int numberOfDrivers)
{
	int i;
    driverRatingInfoGlobal * currentArrayStruct = NULL;

	for (i = 0; i < numberOfDrivers; i++)
	{
		currentArrayStruct = driverInfoArray [i];
		if (currentArrayStruct)
		{
			sumValuesGlobal(currentArrayStruct);
		}
		else
		{ // caso não haja informação do driver em rides, cria uma struct opaca, para não dar erro em funções de sort
			driverInfoArray[i] = newOpaqueDriverInfoGlobal(i + 1);
		}
	}
}

void sumValuesCity (driverRatingInfoCity * currentArrayStruct) {
	unsigned int * ratings = currentArrayStruct->ratings.chart,
                   numRides = 0, j;
    double avgRating = 0.0;

	for (j = 0; j < 5; j++)
	{
		avgRating += ratings[j] * (j + 1);
		numRides += ratings[j];
	}
	avgRating /= numRides;
	currentArrayStruct->ratings.average = avgRating;
}

void sumValuesGlobal (driverRatingInfoGlobal * currentArrayStruct) {
    unsigned int * ratings = currentArrayStruct->ratings.chart,
                   numRides = 0, j;
    double avgRating = 0.0;

	for (j = 0; j < 5; j++)
	{
		avgRating += ratings[j] * (j + 1);
		numRides += ratings[j];
	}
	avgRating /= numRides;
	currentArrayStruct->ridesTracker[0] = numRides; // talvez passar estes bits para o espaço restante no ratings, a seguir do double?
	currentArrayStruct->ratings.average = avgRating;
}

driverRatingInfoGlobal * newOpaqueDriverInfoGlobal(int driverNumber)
{
	driverRatingInfoGlobal * newStruct = malloc(sizeof(driverRatingInfoGlobal));
	newStruct->ratings.average = 0.0;
	newStruct->driverNumber = (short int)driverNumber;
	newStruct->tips = 0.0;
	newStruct->mostRecRideDate = 0; // meter datas a 0 // é preciso definir valor da data??? n chega a testar este parâmetro
	return (newStruct);
}

driverRatingInfoCity * newOpaqueDriverInfoCity(int driverNumber)
{
	driverRatingInfoCity * newStruct = malloc(sizeof(driverRatingInfoCity));
	newStruct->ratings.average = 0.0;
	newStruct->driverNumber = (short int)driverNumber;
	return (newStruct);
}

inline gint compareRidesByDate(gconstpointer a, gconstpointer b)
{
	return (*(RidesStruct **)a)->date - (*(RidesStruct **)b)->date;
}

void freeRidesData(RidesData *data)
{
	RidesData *dataStruct = data;
	int numberOfDrivers = dataStruct->numberOfDrivers;

	// free rides por cidade
	GHashTable *table = dataStruct->cityTable;
	g_hash_table_destroy(table);
	// free info resumida de drivers global, ordenado por ID
	freeRidesRatingGlobal(dataStruct->driverInfoArray, numberOfDrivers);

	// free info resumida de drivers global, ordenada por parâmetros da Q2
	free(dataStruct->driverRatingArray); // não se dá free dos elementos deste array, porque são apenas pointers (para rides que recebem free noutro sítio)

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
			free(currentRideStruct->user);
			free(currentRideStruct->city);
		}
	//free(currentRideStruct->comment);
	}
	free(secondaryArrayStruct);
}

void freeRidesRatingGlobal(driverRatingInfoGlobal ** driversRatings,int numberOfDrivers) {
	int i;
	driverRatingInfoGlobal *currentArrayStruct;
	for (i=0;i < numberOfDrivers;i++) {
	currentArrayStruct = (driverRatingInfoGlobal *)driversRatings[i];
	free(currentArrayStruct);
	}
	free(driversRatings);
}

void freeRidesRatingCity(driverRatingInfoCity ** driversRatings,int numberOfDrivers) {
	int i;
	driverRatingInfoCity *currentArrayStruct;
	for (i=0;i < numberOfDrivers;i++) {
	currentArrayStruct = (driverRatingInfoCity *)driversRatings[i];
	free(currentArrayStruct);
	}
	free(driversRatings);
}

void freeCityRides(void *data) {
	CityRides * cityInfo = (CityRides *)data;
	g_ptr_array_free(cityInfo->cityRidesArray, TRUE);
	freeRidesRatingCity(cityInfo->driverSumArray, cityInfo->numberOfDrivers);
	free(cityInfo);
}

//funções relativas a rides ordenadas por ID de driver
const ridesByDriverGlobal * getRidesByDriverGlobal(const RidesData * ridesData) {// responsabilidade da caller function dar free
	ridesByDriverGlobal * driverInfoArray = malloc(sizeof(ridesByDriverGlobal));
	driverInfoArray->ridesArray = ridesData->driverInfoArray;
	driverInfoArray->len = ridesData->numberOfDrivers;
	return driverInfoArray;
}

//funcções relativas a rides ordenadas

//array com resumo da info global de cada driver, ordenada com parâmetros da Q2
const ridesByDriverGlobal* getRidesByDriverGlobalSorted(const RidesData * ridesData) {
	ridesByDriverGlobal * driverRatingArrayWrapper = malloc(sizeof(ridesByDriverGlobal));
	driverRatingArrayWrapper->ridesArray = ridesData->driverRatingArray;
	driverRatingArrayWrapper->len = ridesData->numberOfDrivers;
	return driverRatingArrayWrapper;
}

//array com resumo da info de cada driver numa dada cidade, ordenada com parâmetros da Q7
const ridesByDriverCity* getRidesByDriverCity(const CityRides * cityRides) {
	ridesByDriverCity * driverInfoArrayWrapper = malloc(sizeof(ridesByDriverCity));
	driverInfoArrayWrapper->ridesArray = cityRides->driverSumArray; // supõe que a key existe sempre, se não for o caso passar a usar g_hash_table_lookup_extended
	driverInfoArrayWrapper->len = cityRides->numberOfDrivers;
	return driverInfoArrayWrapper;
}

//obter a info de um driver, a partir de um array ridesByDriver
const driverRatingInfoGlobal* getDriverInfoGlobal(const ridesByDriverGlobal *ridesByDriver, guint id)
{
	driverRatingInfoGlobal ** array = ridesByDriver->ridesArray;
	// if (id > array->len) return NULL;
	driverRatingInfoGlobal *currentArrayStruct = array [id - 1];
	return (currentArrayStruct);
}

const driverRatingInfoCity* getDriverInfoCity(const ridesByDriverCity *ridesByDriver, guint id)
{
	driverRatingInfoCity ** array = ridesByDriver->ridesArray;
	// if (id > array->len) return NULL;
	driverRatingInfoCity *currentArrayStruct = array [id - 1];
	return (currentArrayStruct);
}

//obter tamanho do array ridesByDriver
int getRidesByDriverGlobalArraySize(const ridesByDriverGlobal *driverInfoArray)
{
	return (driverInfoArray->len);
}

int getRidesByDriverCityArraySize(const ridesByDriverCity * driverInfoArray)
{
	return (driverInfoArray->len);
}

gint sort_byRatings_2 (const void * a, const void * b) {
    const driverRatingInfoGlobal * driver1 = *(driverRatingInfoGlobal **) a;
    const driverRatingInfoGlobal * driver2 = *(driverRatingInfoGlobal **) b;
    double diff = driver1->ratings.average - driver2->ratings.average;	
    gint result = 0;
    if (diff > 0) result = 1;
    else if (diff <0) result = -1;
    else if (!result && driver1->ratings.average) {  // previne comparações entre nodos de riders com rating 0 (não apareciam nas rides)
        result = compDates(driver1->mostRecRideDate, driver2->mostRecRideDate);
        if (!result) 
            result = driver2->driverNumber - driver1->driverNumber; // comparação em ordem crescente do ID, 2 - 1, porque leitura final é feita do fim para o início
    }
    return result;
}

gint sort_byRatings_7 (const void * a, const void * b) {
    const driverRatingInfoCity * driver1 = *(driverRatingInfoCity **) a;
    const driverRatingInfoCity * driver2 = *(driverRatingInfoCity **) b;
    double diff = driver1->ratings.average - driver2->ratings.average;	
    gint result = 0;
    if (diff > 0) result = 1;
    else if (diff <0) result = -1;
    else if (!result && driver1->ratings.average) {  // previne comparações entre nodos de riders com rating 0 (não apareciam nas rides)
		result = driver1->driverNumber - driver2->driverNumber; // comparação em ordem decrescente do ID, 1 - 2, porque leitura final é feita do fim para o início
	}
    return result;
}

driverRatingInfoGlobal ** buildRidesbyDriverSorted (driverRatingInfoGlobal ** driverInfoArray, int numberOfDrivers) {
	driverRatingInfoGlobal ** driverRatingArray = calloc(numberOfDrivers,sizeof(driverRatingInfoGlobal *)); // copia leve do outro array, só queremos copiar os pointers e ordenar os pointers, não se mexe nas rides em si
	int i;
	for (i=0; i<numberOfDrivers;i++) {
		driverRatingArray[i] = driverInfoArray[i]; 
	}
	qsort(driverRatingArray, numberOfDrivers, sizeof(driverRatingInfoGlobal *),sort_byRatings_2);
	return driverRatingArray;
}

//## usado só quando se tem multithreading!!
//constroi o resumo da info de um driver para um dado conjunto de rides (utilizado em rides de cada cidade, daí o nome de momento)
driverRatingInfoCity ** buildRidesByDriverCity(GPtrArray * ridesInCity, int numberOfDrivers) {
	gint i, numberOfRides = ridesInCity->len; // já é dinâmico
	driverRatingInfoCity * * driverInfoArray = calloc(numberOfDrivers,sizeof(driverRatingInfoCity *));
	for (i=0;i<numberOfRides;i++) {
		addDriverInfoCity(driverInfoArray, g_ptr_array_index(ridesInCity, i));
	}
	//driverInfoArray = getPresentableValues(driverInfoArray);
	//g_ptr_array_sort(driverInfoArray,sort_byRatings_7);
	return driverInfoArray;
}

//a diferença em relação a newDriverGlobalInfo é receber uma struct de tipo diferente, daí ter contas um pouco diferentes
driverRatingInfoGlobal * newCopyDriverGlobal(driverRatingInfoCity *currentDriverInfo) {
	driverRatingInfoGlobal * newStruct = malloc(sizeof(driverRatingInfoGlobal));
	memcpy(newStruct->ratings.chart,currentDriverInfo->ratings.chart,sizeof(unsigned int) * 5);
	memset(newStruct->ridesTracker,0,sizeof(short int) * 2);
	newStruct->driverNumber = currentDriverInfo->driverNumber;
	newStruct->mostRecRideDate = 0;
	newStruct->tips = 0.0;
	return newStruct;		
}

//a diferença em relação a appenDriverGlobalInfo é receber uma struct de tipo diferente, daí ter contas um pouco diferentes
void appendCopyDriverGlobal(driverRatingInfoGlobal * currentGlobalArrayStruct, driverRatingInfoCity * currentCityArrayStruct) {
	short int i;
	unsigned int * ratingsTo, * ratingsFrom;
	ratingsTo = (unsigned int *)currentGlobalArrayStruct->ratings.chart;
	ratingsFrom = (unsigned int *)currentCityArrayStruct->ratings.chart;
	for (i=0;i<5;i++) {
		ratingsTo[i] += ratingsFrom[i];
	}
}

// organizar estruturas relativas a cada cidade: ordenar rides por data, ordenar info de drivers por parâmetros da Q2
void OrderStatisticsInCity(CityRides *city, int numberOfDrivers) {
	g_ptr_array_sort(city->cityRidesArray, compareRidesByDate); // ordenar rides por data (usado nas Q4,Q5,Q6,Q7,Q9)
	qsort(city->driverSumArray, numberOfDrivers, sizeof(driverRatingInfoCity *), sort_byRatings_7); // ordenação da info (resumida antes) sobre drivers de uma cidade (usado na query 7)
}

// obtém médias globais de cada driver
// COMO: pega nas informações acumuladas de cada driver de cada cidade e adiciona-a à info de cada driver global
// aproveita-se o loop desta função para: calcular as médias de cada driver em cada cidade e dar sort de cada cidade segundo os parâmetro da Q7
driverRatingInfoGlobal ** buildRidesByDriverGlobal(GHashTable * cityTable, driverRatingInfoGlobal ** driverInfoGlobalArray, int numberOfDrivers) {
	int j;
	driverRatingInfoCity * currentCityArrayStruct;
	driverRatingInfoGlobal * currentGlobalArrayStruct;

	//criar structs com valores a 0 em cada posição, para acumular valores nessa posição depois
	// !! outra forma mais eficiente sem meter NULL com set_size?
	GHashTableIter iter;
	gpointer value;

	g_hash_table_iter_init (&iter, cityTable);
	while(g_hash_table_iter_next (&iter, NULL, &value)){ // para cada cidade
		CityRides * cityData = (CityRides * ) value;
		driverRatingInfoCity ** driverInfoInCityArray = cityData->driverSumArray;
		for (j=0;j<numberOfDrivers;j++) { // para cada driver de uma cidade
				currentGlobalArrayStruct = driverInfoGlobalArray [j];
				currentCityArrayStruct = driverInfoInCityArray[j];
			if (currentCityArrayStruct != NULL) { // se o driver tiver informação numa dada cidade
				if (currentGlobalArrayStruct == NULL) {  // se no array global ainda não ha informação desse driver
					driverInfoGlobalArray[j] = newCopyDriverGlobal(currentCityArrayStruct); // cria uma estrutura nova com informação desse driver
				} else { // se no array global já havia informação desse driver
					appendCopyDriverGlobal(currentGlobalArrayStruct,currentCityArrayStruct); // acrescenta ao array global a informação desse driver
				}
				sumValuesCity(currentCityArrayStruct); // aproveita-se o loop para calcular médias finais do driver de uma cidade
			}
			else {
				driverInfoInCityArray[j] = newOpaqueDriverInfoCity(j + 1); // aproveita-se o loop para construir uma estrutura opaca para drivers NULL (necessário para o sort)
			}
		}
		// aproveita-se o loop para organizar estruturas relativas a cada cidade
		OrderStatisticsInCity(cityData,numberOfDrivers);
	}

	// cálculo das médias finais do driver de uma cidade, só possível no fim do loop anterior, quando toda a informação é obtida
	getPresentableValuesGlobal(driverInfoGlobalArray, numberOfDrivers); 
	return driverInfoGlobalArray;
}

inline double getDriverGlobalAvgRating(const driverRatingInfoGlobal *currentArrayStruct)
{
	return (currentArrayStruct->ratings.average);
}

//repetição
inline double getDriverCityAvgRating(const driverRatingInfoCity *currentArrayStruct)
{
	return (currentArrayStruct->ratings.average);
}

inline double getDriverTipsTotal(const driverRatingInfoGlobal *currentArrayStruct)
{
	return (currentArrayStruct->tips);
}

inline Date getDriverMostRecRideDate(const driverRatingInfoGlobal * currentArrayStruct)
{
	return currentArrayStruct->mostRecRideDate;
}

inline short int getDriverRidesNumber(const driverRatingInfoGlobal *currentArrayStruct)
{
	return (currentArrayStruct->ridesTracker[0]);
}

inline short int getDriverDistTraveled(const driverRatingInfoGlobal *currentArrayStruct)
{
	return (currentArrayStruct->ridesTracker[1]);
}

inline int getDriverNumberGlobal(const driverRatingInfoGlobal *currentArrayStruct)
{
	return currentArrayStruct->driverNumber;
}

inline int getDriverNumberCity(const driverRatingInfoCity *currentArrayStruct)
{
	return currentArrayStruct->driverNumber;
}

// funções relativas a rides ordenadas por cidade
inline const CityRides *getRidesByCity(const RidesData *data, const char *city) // responsabilidade da caller function dar free
{
	return g_hash_table_lookup(data->cityTable, city);
}

inline guint getNumberOfCityRides(const CityRides *rides)
{
	return rides->cityRidesArray->len;
}

inline const RidesStruct *getCityRidesByIndex(const CityRides *rides, guint ID)
{
	return (RidesStruct *)g_ptr_array_index(rides->cityRidesArray, (int)ID);
}

void iterateOverCities(const RidesData *rides, void *data, void (*iterator_func)(const CityRides *, void *))
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
int custom_bsearch(GPtrArray *array, Date date, int mode) {
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

void searchCityRidesByDate(const CityRides * cityRides, Date dateA, Date dateB, int *res_start, int *res_end) {
	GPtrArray *array = cityRides->cityRidesArray;
	int len = array->len;
	Date first, last;
	RidesStruct *ride;
	ride = g_ptr_array_index(array, 0);
	first = ride->date;
	ride = g_ptr_array_index(array, len - 1);
	last = ride->date;

	// printf("-------------------------------\nRequested dates: %d/%d/%hd %d/%d/%hd\n", dateA->day,dateA->month,dateA->year, dateB->day,dateB->month,dateB->year);
	// printf("First: %d/%d/%hd last: %d/%d/%hd\n", first->day,first->month,first->year, last->day,last->month,last->year);

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
	// DATE dateStart = ((RidesStruct *)g_ptr_array_index(array, *res_start))->date;
	// DATE dateEnd = ((RidesStruct *)g_ptr_array_index(array, *res_end))->date;
	// printf("these correspond to %d/%d/%hd %d/%d/%hd\n", dateStart.day,dateStart.month,dateStart.year,dateEnd.day,dateEnd.month,dateEnd.year);
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
				fprintf(fp, "%s iter%d | date:%d/%d/%d, driver: %d, distance: %d\n", ride->city,j, GET_DATE_DAY(ride->date),\
					GET_DATE_MONTH(ride->date), GET_DATE_YEAR(ride->date), ride->driver, ride->distance);
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
			fprintf(fp, "%s iter%d | date:%d/%d/%d, driver: %d, distance: %d\n",ride->city,i, GET_DATE_DAY(ride->date),\
				GET_DATE_MONTH(ride->date), GET_DATE_YEAR(ride->date), ride->driver, ride->distance);
		}
	fclose(fp);
	}
}

//dump driverInfoArray: dataState == 0 -> before getPresentablevalues, dataState == 1 -> after getPresentablevalues!!
void dumpDriverInfoArray (char * filename, GPtrArray * driverInfo, char * addToFilename, int dataState) {
	if(addToFilename != NULL) filename = strcat(filename,addToFilename);
	FILE *fp = fopen(filename, "w");
	int i, numberOfDrivers = driverInfo->len;
	driverRatingInfoGlobal * currentDriver;
	for (i = 0; i < numberOfDrivers; i++) {
		currentDriver = g_ptr_array_index(driverInfo, i);
		if (currentDriver == NULL) fprintf(fp,"driverNumber:%d | NO INFO!\n",i+1);
		else {
			if (!dataState) {
			unsigned int * ratings = currentDriver->ratings.chart;
			//NumberOfRides não foi calculado ainda, só depois de getPresentableValues, por isso não é incluído no print
			fprintf(fp, "driverNumber:%d, mostRecdate: %d/%d/%u, avgRatings: [%d,%d,%d,%d,%d], totalTravalled:%d\n",\
				currentDriver->driverNumber, GET_DATE_DAY(currentDriver->mostRecRideDate), GET_DATE_MONTH(currentDriver->mostRecRideDate),\
				GET_DATE_YEAR(currentDriver->mostRecRideDate), ratings[0], ratings[1], ratings[2], ratings[3], ratings[4], currentDriver->ridesTracker[1]);
			}
			else {
				fprintf(fp,"driverNumber:%d,mostRecdate: %d/%d/%u, avgRating:%f, ridesN:%u, distTotal:%u\n",currentDriver->driverNumber,\
					GET_DATE_DAY(currentDriver->mostRecRideDate), GET_DATE_MONTH(currentDriver->mostRecRideDate),GET_DATE_YEAR(currentDriver->mostRecRideDate),\
					currentDriver->ratings.average, currentDriver->ridesTracker[0], currentDriver->ridesTracker[1]);
			}
		}
	}
	fclose(fp);
}

// devolve a struct(dados) associada à ride número i
// esta função nunca é chamada????
RidesStruct * getRidePtrByID(const RidesData *data, guint ID)
{
	exit(5);
	// ID -= 1; // para a primeira ride passar a ser 0
	// guint i = ID / SIZE;
	// GPtrArray *array = data->ridesArray;
	// SecondaryRidesArray *secondaryArray = g_ptr_array_index(array, i);

	// RidesStruct * result = &(secondaryArray->ridesArray[ID - SIZE * i]);
	// return (RIDE_IS_VALID(result)) ? result : NULL;
}

inline int getRideID(const RidesStruct * ride) {
	return ride->ID;
}

inline Date getRideDate(const RidesStruct *ride){
	return ride->date;
}

inline int getRideDriver(const RidesStruct *ride) {
	return (ride->driver);
}

//ver o encapsulamente depois!
inline char * getRideUser(const RidesStruct *ride) {
	return strndup(ride->user, RIDE_STR_BUFF);
}

inline char *  getRideCity(const RidesStruct *ride) {
	return strndup(ride->city, RIDE_STR_BUFF);
}

inline short int getRideDistance(const RidesStruct *ride) {
	return (ride->distance);
}

inline short int getRideScore_u(const RidesStruct *ride) {
	return (ride->score_u);
}

inline short int getRideScore_d(const RidesStruct *ride) {
	return (ride->score_d);
}

inline float getRideTip(const RidesStruct *ride) {
	return (ride->tip);
}

// char *getRideComment(RidesStruct *ride)
// {
// 	return strndup(ride->comment, RIDE_STR_BUFF);
// }

inline int rideIsValid(const RidesStruct *ride) {
	return (ride != NULL && RIDE_IS_VALID(ride));
}

int getNumberOfRides(const RidesData * data) {
	GPtrArray * array = data->ridesArray;
	int num = (array->len - 1) * SIZE;
	SecondaryRidesArray * secondaryArray = g_ptr_array_index(array, array->len - 1);
	num += secondaryArray->len;
	return num;
}

inline const int *getRidesDistance(const CityRides *rides) {
	return rides->distance;
}

inline const int *getRidesTotal(const CityRides *rides) {
	return rides->total;
}
