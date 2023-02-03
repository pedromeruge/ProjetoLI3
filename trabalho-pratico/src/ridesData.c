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

//union relativa a ratings de drivers: guarda um array de ratings de 1 a 5, e mais tarde o valor médio desses ratings
typedef union { 
	unsigned int chart[5];
	double average;
} ratingsUnion;

//NOTA: ratings e driverNumber têm de aparecer nestas posições
struct fullDriverInfo {
	ratingsUnion ratings;
	int driverNumber; // tem de ser int, em large-dataset temos drivers até 100.000
	short int ridesNumber,
			totalTravel;
    double tips; // inicialmente guarda número de tips, depois guarda o total_auferido
    Date mostRecRideDate; // ride mais recente
};

// NOTA: ratings e driverNumber têm de aparecer nestas posições)
struct partialDriverInfo {
	ratingsUnion ratings;
	int driverNumber; // tem de ser int, em large-dataset temos drivers até 100.000
};

// typedef struct {
// 	GThread * thread;
// 	CityRides **cityRidesPtrArray;
// 	int len,
// 	n_of_drivers;
// } ThreadStruct;

// cada key da GHashTable cityTable aponta para uma struct deste tipo
// apenas entram nas cityRides entradas válidas, já que não se faz get por ID
struct CityRides {
	GPtrArray * cityRidesArray;
	partialDriverInfo * driverSumArray; // array com informação resumida de cada driver, para uma dada cidade
	int distance[3], total[3];
}; 

typedef struct {
	int len;
	RidesStruct ridesArray[SIZE]; // já sabemos que vai levar malloc de SIZE e já, e assim usamos g_ptr_array
} SecondaryRidesArray;

struct RidesData {
	GPtrArray * ridesArray;
	GHashTable *cityTable;
	fullDriverInfo * driverInfoArray; // info de drivers ordenada por ID
	fullDriverInfo ** driverRatingArray; // info de drivers ordenada por parâmetros da Q2
	int numberOfDrivers; // para saber o tamanho dos arrays no freeRidesData
	GArray * femaleArray;
	GArray * maleArray;
};

void freeCityRides(void *data);
gint compareRidesByDate(gconstpointer, gconstpointer);
void *buildStatisticsInCity(void *);
SecondaryRidesArray *getRides(FILE *ptr, GHashTable *cityTable, const parse_format *format, int *invalid, const UserData *userdata,\
const DriverData *driverdata, fullDriverInfo * driverArray, int numberOfDrivers, int *bp, int *sp, char *buffer, GArray * maleArray, GArray * femaleArray);

fullDriverInfo ** buildRidesbyDriverSorted (fullDriverInfo *, int);
partialDriverInfo * * buildRidesByDriverCity(GPtrArray *, int);
void buildRidesByDriverGlobal(GHashTable *, fullDriverInfo *, int);

void addDriverInfoCity(partialDriverInfo *,RidesStruct *);
void addDriverInfoGlobal(fullDriverInfo *,RidesStruct *);
void appendDriverInfoCity (partialDriverInfo *, RidesStruct *);
void getPresentableValues (fullDriverInfo **, int);
void sumValuesCity (partialDriverInfo *, int);
void sumValuesGlobal (fullDriverInfo *);

void freeRidesPtrArray (void *);
void freeRidesRatingCity(partialDriverInfo **, int);

void dumpDriversInfoArray (char * filename, fullDriverInfo * driverInfo, char * addToFilename, int dataState, int numberOfDrivers);
void dumpDriversRatingArray (char * filename, partialDriverInfo ** driverInfo, char * addToFilename, int dataState, int numberOfDrivers);

RidesData * getRidesData(FILE *ptr, const UserData *userdata, const DriverData *driverdata, char *buffer) {

	RidesData *data = malloc(sizeof(RidesData));

	GArray * maleArray = new_gender_array(); 
	GArray * femaleArray = new_gender_array();

	int numberOfDrivers = getNumberOfDrivers(driverdata); // número de drivers, necessário para construir arrays com info de cada driver (Q1,Q2,Q7)

	//inicializar as estruturas de dados relacionadas com as rides lidas do ficheiro diretamente
	GPtrArray * ridesArray = g_ptr_array_new_with_free_func(freeRidesPtrArray);
	SecondaryRidesArray * secondaryArray;

	//inicializa uma hash table
	GHashTable *cityTable = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, freeCityRides); // keys levam malloc do array normal, nao vou dar free aqui;
	
	//inicializa um array com info de drivers global, é um array (sem pointers) porque não leva sort, há outro array que aponta para este, e esse é que é sorted, não este!
	fullDriverInfo * driverInfoGlobalArray = calloc(numberOfDrivers,sizeof(fullDriverInfo)); // calloc inicial do array, para evitar checks depois
	if (driverInfoGlobalArray == NULL) fprintf(stderr,"Não consegui alocar o array");
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

	secondaryArray = getRides(ptr, cityTable, &format, &invalid, userdata, driverdata, driverInfoGlobalArray, numberOfDrivers, &bp, &sp, buffer, femaleArray, maleArray);

	while (secondaryArray != NULL) {
		g_ptr_array_add(ridesArray, secondaryArray);
		secondaryArray = getRides(ptr, cityTable, &format, &invalid, userdata, driverdata, driverInfoGlobalArray, numberOfDrivers, &bp, &sp, buffer, femaleArray, maleArray);
	}

	int num = (ridesArray->len - 1) * SIZE;
	secondaryArray = g_ptr_array_index(ridesArray, ridesArray->len - 1);
	num += secondaryArray->len;
	printf("Total number of rides: %d\nNumber of invalid rides: %d\n", num, invalid);

	//## MT para cálulco de estatística de dados para cada cidade
	// assumimos que o nº de cidades esta bem distribuido (na fase 1 pelo menos estava)
	//multiThreadedCityRides(cityTable, driverdata, numberOfDrivers);
	
	buildRidesByDriverGlobal(cityTable,driverInfoGlobalArray, numberOfDrivers);

	data->numberOfDrivers = numberOfDrivers; // usado para o freeRidesArray
	data->ridesArray = ridesArray; // array com input do ficheiro
	data->cityTable = cityTable; // hash table que guarda structs com: array com rides ordenandas para uma cidade key, e array com info resumida de drivers nessa cidade;
	data->driverInfoArray = driverInfoGlobalArray; // array com informação de drivers resumida global
	data->driverRatingArray = buildRidesbyDriverSorted(driverInfoGlobalArray,numberOfDrivers);
	sort_userarray(userdata);
	//dumpDriverInfoArray("query_2-ouputs",data->driverInfoArray,NULL,1, numberOfDrivers);
	return data;
}

SecondaryRidesArray *getRides(FILE *ptr, GHashTable *cityTable, const parse_format *format, int *invalid, const UserData *userdata,\
const DriverData *driverdata, fullDriverInfo * driverArray, int numberOfDrivers, int *bp, int *sp, char *buffer, GArray * maleArray, GArray * femaleArray) {
	
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

			add_gender_info(maleArray, femaleArray, getDriverPtrByID(driverdata, temp->driver), getUserPtrByUsername(userdata, temp->user), temp->ID);

			// check if city is not already in hash table
			if ((cityRides = g_hash_table_lookup(cityTable, city)) == NULL)
			{
				// if not, insert
				cityRides = malloc(sizeof(CityRides));
				cityRides->cityRidesArray = g_ptr_array_sized_new(RIDES_PER_CITY);
				cityRides->driverSumArray = calloc(numberOfDrivers,sizeof(partialDriverInfo)); // array com info de drivers sobre a cidade específica; assume-se que NULL corresponde a tudo 0s!!!

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

//adiciona driverRatingInfo sobre tips, distância percorrida e data mais recente, no global (Q1,Q2)
void addDriverInfoGlobal(fullDriverInfo * driverInfoArray, RidesStruct *currentRide) {
	fullDriverInfo * currentArrayStruct = &(driverInfoArray [currentRide->driver - 1]);
	currentArrayStruct->tips += currentRide->tip;
	// currentArrayStruct.ratings.chart[(currentRide->score_d) - 1]++; // cada cidade calcula isto, na construção final deste array apenas soma-se os valores de cada cidade, em vez de calcular aqui
	currentArrayStruct->totalTravel += currentRide->distance;
	//comparar se novas inserções têm datas mais recentes
	if (compDates(currentArrayStruct->mostRecRideDate,currentRide->date) < 0)
		currentArrayStruct->mostRecRideDate = currentRide->date;
	//currentArrayStruct->driverNumber = currentRide->driver // esta atribuição é feita apenas no loop final, para não ser repetida aqui e lá
}

//adiciona driverRatingInfo de uma ride, numa cidade (Q7)
void addDriverInfoCity(partialDriverInfo * driverInfoArray, RidesStruct *currentRide) {
	(driverInfoArray [currentRide->driver - 1]).ratings.chart[(currentRide->score_d) - 1]++;
	//currentArrayStruct->driverNumber = currentRide->driver // esta atribuição é feita apenas no loop final, para não ser repetida aqui e lá
}

void getPresentableValuesGlobal(fullDriverInfo * driverInfoArray, int numberOfDrivers)
{
	int i, j;
	unsigned int * ratings, numRides = 0;
    double avgRating = 0.0;
    fullDriverInfo * currentArrayStruct;

	for (i = 0; i < numberOfDrivers; i++) {
		numRides = 0;
		avgRating = 0.0;
		currentArrayStruct = &driverInfoArray[i];
		ratings = currentArrayStruct->ratings.chart;
		for (j = 0; j < 5; j++) {
			avgRating += ratings[j] * (j + 1);
			numRides += ratings[j];
		}
	currentArrayStruct->ridesNumber = numRides; // talvez passar estes bits para o espaço restante no ratings, a seguir do double?
	if (avgRating != 0) avgRating /= numRides; // só se divide a avg, se não for zero, ou daria Nan!
	currentArrayStruct->ratings.average = avgRating;
	currentArrayStruct->driverNumber = i+1;
	//printf("driverNumber:%d, rating:%f\n",currentArrayStruct->driverNumber,currentArrayStruct->ratings.average);
	}
}

void sumValuesCity (partialDriverInfo * currentArrayStruct, int driverNumber) {
	currentArrayStruct->driverNumber = driverNumber;
	unsigned int * ratings = currentArrayStruct->ratings.chart,
                   numRides = 0, j;
    double avgRating = 0.0;

	for (j = 0; j < 5; j++)
	{
		avgRating += ratings[j] * (j + 1);
		numRides += ratings[j];
	}
	if (avgRating != 0) avgRating /= numRides; // se a avg se mantiver como 0, não fazer conta, daria Nan!
	currentArrayStruct->ratings.average = avgRating;

}


inline gint compareRidesByDate(gconstpointer a, gconstpointer b)
{
	return (*(RidesStruct **)a)->date - (*(RidesStruct **)b)->date;
}

void freeRidesData(RidesData *data)
{
	RidesData *dataStruct = data;

	// free rides por cidade
	GHashTable *table = dataStruct->cityTable;
	g_hash_table_destroy(table);

	// free info resumida de drivers global, ordenado por ID
	free(dataStruct->driverInfoArray);

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

void freeCityRides(void *data) {
	CityRides * cityInfo = (CityRides *)data;
	g_ptr_array_free(cityInfo->cityRidesArray, TRUE);
	free(cityInfo->driverSumArray);
	free(cityInfo);
}


//obter a info global de um driver, a partir de um ID
const fullDriverInfo * getDriverGlobalInfoByID(const RidesData * data, unsigned int id)
{
	return (&(data->driverInfoArray[id-1])); 
}

//obter a info parcial de um driver, a partir de um ID, ordenado por parâmetros da Q2
const partialDriverInfo * getDriverGlobalRatingByID(const void * ridesData, unsigned int id) {
	const RidesData * data = (RidesData *) ridesData;
	return ((partialDriverInfo *) data->driverRatingArray[id-1]); // as duas structs têm os mesmo argumentos iniciais !!
}

//obter a info parcial de um driver, a partir de um ID, ordenado por parâmetros da Q7
const partialDriverInfo * getDriverCityRatingByID(const void * cityData, unsigned int id) {
	const CityRides * data = (CityRides *) cityData;
	return (&(data->driverSumArray[id-1]));
}

gint sort_byRatings_2 (const void * a, const void * b) {
    const fullDriverInfo * driver1 = *(fullDriverInfo **) a;
    const fullDriverInfo * driver2 = *(fullDriverInfo **) b;
    double diff = driver2->ratings.average - driver1->ratings.average;	
    gint result = 0;
    if (diff > 0) result = 1;
    else if (diff <0) result = -1;
    else if (!result && driver1->ratings.average) {  // previne comparações entre nodos de riders com rating 0 (não apareciam nas rides)
        result = compDates(driver2->mostRecRideDate, driver1->mostRecRideDate);
        if (!result) 
            result = driver1->driverNumber - driver2->driverNumber; // comparação em ordem crescente do ID, 2 - 1, porque leitura final é feita do fim para o início
    }
    return result;
}

gint sort_byRatings_7 (const void * a, const void * b) {
    const partialDriverInfo driver1 = *(partialDriverInfo *) a;
    const partialDriverInfo driver2 = *(partialDriverInfo *) b;
    double diff = driver2.ratings.average - driver1.ratings.average;	
    gint result = 0;
    if (diff > 0) result = 1;
    else if (diff <0) result = -1;
    else if (!result && driver1.ratings.average) {  // previne comparações entre nodos de riders com rating 0 (não apareciam nas rides)
		result = driver2.driverNumber - driver1.driverNumber; // comparação em ordem decrescente do ID, 1 - 2, porque leitura final é feita do fim para o início
	}
    return result;
}

fullDriverInfo ** buildRidesbyDriverSorted (fullDriverInfo * driverInfoArray, int numberOfDrivers) {
	fullDriverInfo ** driverRatingArray = malloc(numberOfDrivers * sizeof(fullDriverInfo *)); // copia leve do outro array, só queremos copiar os pointers e ordenar os pointers, não se mexe nas rides em si
	int i;
	fullDriverInfo * offsetArray = driverInfoArray;
	for (i=0; i<numberOfDrivers;i++, offsetArray++) {
		driverRatingArray[i] = offsetArray;
	}
	qsort(driverRatingArray, numberOfDrivers, sizeof(fullDriverInfo *),sort_byRatings_2);
	return driverRatingArray;
}

//a diferença em relação a appendDriverGlobalInfo é receber uma struct de tipo diferente, daí ter contas um pouco diferentes
void copyDriverGlobal(fullDriverInfo * currentGlobalArrayStruct, partialDriverInfo * currentCityArrayStruct) {
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
	qsort(city->driverSumArray, numberOfDrivers, sizeof(partialDriverInfo), sort_byRatings_7); // ordenação da info (resumida antes) sobre drivers de uma cidade (usado na query 7)
}

// obtém médias globais de cada driver
// COMO: pega nas informações acumuladas de cada driver de cada cidade e adiciona-a à info de cada driver global
// aproveita-se o loop desta função para: calcular as médias de cada driver em cada cidade e dar sort de cada cidade segundo os parâmetro da Q7
void buildRidesByDriverGlobal(GHashTable * cityTable, fullDriverInfo * driverInfoGlobalArray, int numberOfDrivers) {
	int j;
	partialDriverInfo * currentCityArrayStruct;
	fullDriverInfo * currentGlobalArrayStruct;

	//criar structs com valores a 0 em cada posição, para acumular valores nessa posição depois
	// !! outra forma mais eficiente sem meter NULL com set_size?
	GHashTableIter iter;
	gpointer value;

	g_hash_table_iter_init (&iter, cityTable);
	while(g_hash_table_iter_next (&iter, NULL, &value)){ // para cada cidade
		CityRides * cityData = (CityRides * ) value;
		partialDriverInfo * driverInfoInCityArray = cityData->driverSumArray;
		for (j=0;j<numberOfDrivers;j++) { // para cada driver de uma cidade
				currentGlobalArrayStruct = &driverInfoGlobalArray [j];
				currentCityArrayStruct = &driverInfoInCityArray[j];
				copyDriverGlobal(currentGlobalArrayStruct,currentCityArrayStruct); // acrescenta ao array global a informação desse driver
				sumValuesCity(currentCityArrayStruct, j+1); // aproveita-se o loop para calcular médias finais do driver de uma cidade
		}
		// aproveita-se o loop para organizar estruturas relativas a cada cidade
		OrderStatisticsInCity(cityData,numberOfDrivers);
		//dumpDriversRatingArray("query_7-ouputs",cityData->driverSumArray,NULL,1, numberOfDrivers);
	}

	// cálculo das médias finais do driver de uma cidade, só possível no fim do loop anterior, quando toda a informação é obtida
	getPresentableValuesGlobal(driverInfoGlobalArray, numberOfDrivers); 
}

inline double getDriverGlobalAvgRating(const fullDriverInfo *currentArrayStruct)
{
	return (currentArrayStruct->ratings.average);
}


inline double getDriverTipsTotal(const fullDriverInfo *currentArrayStruct)
{
	return (currentArrayStruct->tips);
}

inline Date getDriverMostRecRideDate(const fullDriverInfo * currentArrayStruct)
{
	return currentArrayStruct->mostRecRideDate;
}

inline short int getDriverRidesNumber(const fullDriverInfo *currentArrayStruct)
{
	return (currentArrayStruct->ridesNumber);
}

inline short int getDriverDistTraveled(const fullDriverInfo *currentArrayStruct)
{
	return (currentArrayStruct->totalTravel);
}

inline int getDriverNumberGlobal(const fullDriverInfo *currentArrayStruct)
{
	return currentArrayStruct->driverNumber;
}

inline double getDriverAvgRating(const partialDriverInfo *currentArrayStruct)
{
	return (currentArrayStruct->ratings.average);
}

inline int getDriverNumber(const partialDriverInfo *currentArrayStruct)
{
	return currentArrayStruct->driverNumber;
}

int getDriversRatingArraySize(const RidesData * RidesData)
{
	return (RidesData->numberOfDrivers);
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

//dump drivesrInfoArray: dataState == 0 -> before getPresentablevalues, dataState == 1 -> after getPresentablevalues!!
void dumpDriversInfoArray (char * filename, fullDriverInfo * driverInfo, char * addToFilename, int dataState, int numberOfDrivers) {
	if(addToFilename != NULL) filename = strcat(filename,addToFilename);
	FILE *fp = fopen(filename, "w");
	int i;
	fullDriverInfo * currentDriver;
	for (i = 0; i < numberOfDrivers; i++) {
		currentDriver = &driverInfo[i];
		if (currentDriver->ratings.average == 0) fprintf(fp,"driverNumber:%d | NO INFO!\n",i+1);
		else {
			if (!dataState) {
			unsigned int * ratings = currentDriver->ratings.chart;
			//NumberOfRides não foi calculado ainda, só depois de getPresentableValues, por isso não é incluído no print
			fprintf(fp, "driverNumber:%d, mostRecdate: %d/%d/%u, avgRatings: [%d,%d,%d,%d,%d], totalTravalled:%d\n",\
				currentDriver->driverNumber, GET_DATE_DAY(currentDriver->mostRecRideDate), GET_DATE_MONTH(currentDriver->mostRecRideDate),\
				GET_DATE_YEAR(currentDriver->mostRecRideDate), ratings[0], ratings[1], ratings[2], ratings[3], ratings[4], currentDriver->totalTravel);
			}
			else {
				fprintf(fp,"driverNumber:%d,mostRecdate: %d/%d/%u, avgRating:%f, ridesN:%u, distTotal:%u\n",currentDriver->driverNumber,\
					GET_DATE_DAY(currentDriver->mostRecRideDate), GET_DATE_MONTH(currentDriver->mostRecRideDate),GET_DATE_YEAR(currentDriver->mostRecRideDate),\
					currentDriver->ratings.average, currentDriver->ridesNumber, currentDriver->totalTravel);
			}
		}
	}
	fclose(fp);
}

void dumpDriversRatingArray (char * filename, partialDriverInfo ** driverInfo, char * addToFilename, int dataState, int numberOfDrivers) {
	if(addToFilename != NULL) filename = strcat(filename,addToFilename);
	FILE *fp = fopen(filename, "w");
	int i;
	partialDriverInfo * currentDriver;
	for (i = 0; i < numberOfDrivers; i++) {
		currentDriver = driverInfo[i];
		if (currentDriver->ratings.average == 0) fprintf(fp,"driverNumber:%d | NO INFO!\n",i+1);
		else {
			if (!dataState) {
			unsigned int * ratings = currentDriver->ratings.chart;
			//NumberOfRides não foi calculado ainda, só depois de getPresentableValues, por isso não é incluído no print
			fprintf(fp, "driverNumber:%d, avgRatings: [%d,%d,%d,%d,%d]\n",\
				currentDriver->driverNumber, ratings[0], ratings[1], ratings[2], ratings[3], ratings[4]);
			}
			else {
				fprintf(fp,"driverNumber:%d,avgRating:%f\n",\
				currentDriver->driverNumber,currentDriver->ratings.average);
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
