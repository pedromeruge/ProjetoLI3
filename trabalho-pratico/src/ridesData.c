#include "ridesData.h"

#define SIZE 1000

struct CityRides {
	GPtrArray *array;
	guint len;
};

void *sortCity(void *data) {
	GPtrArray *array = *(GPtrArray **)data;
	g_ptr_array_sort(array, compareRidesByDate);
	g_thread_exit(NULL);
	return NULL;
}

DATA getRidesData(FILE *ptr) {
	RidesStruct **ridesData = malloc(RIDES_ARR_SIZE*sizeof(RidesStruct *));
	GHashTable *cityTable = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, freeArray); // keys levam malloc do array normal, nao vou dar free aqui
	int i;
	while (fgetc(ptr) != '\n'); // avançar a primeira linha (tbm podia ser um seek hardcoded)
	for (i = 0; i < RIDES_ARR_SIZE; i++) ridesData[i] = getRides(ptr, cityTable);

	RidesData *data = malloc(sizeof(RidesData));
	data->ridesArray = ridesData;
	data->cityTable = cityTable;
	

	// MUDAR ISTO PARA ITERATOR SOBRE A HASHTABLE !!!!!!!!!!!!!!!!!!!!!!!!!!
	guint len;
	const gchar ** cities = (const gchar **)g_hash_table_get_keys_as_array(cityTable, &len);
	GThread *threads[len];
	gpointer args[len];

	for (i = 0; i < (const int)len; i++) {
		args[i] = g_hash_table_lookup(cityTable, cities[i]);
		threads[i] = g_thread_new(NULL, sortCity, &args[i]);
		// sortCity(&args[i]);
	}
	for (i = 0; i < (const int)len; i++) g_thread_join(threads[i]);
	free(cities);

	return data;
}

RidesStruct * getRides(FILE *ptr, GHashTable *cityTable) {
	int i, count, chr;
	char tempBuffer[16], *city;
	GPtrArray *array;
	RidesStruct *ridesStructArray = malloc(SIZE*sizeof(RidesStruct)), *temp;
	for (i = count = 0; i < SIZE; i++, count++) {
		while ((chr = fgetc(ptr)) != ';');// && chr != -1); // skip id
		ridesStructArray[i].date = loadString(ptr);
		writeString(ptr, tempBuffer);
		ridesStructArray[i].driver = (short) atoi(tempBuffer);
		ridesStructArray[i].user = loadString(ptr);
		city = loadString(ptr);
		ridesStructArray[i].city = city;
		writeString(ptr, tempBuffer);
		ridesStructArray[i].distance = (short) atoi(tempBuffer);
		writeString(ptr, tempBuffer);
		ridesStructArray[i].score_u = (short) atoi(tempBuffer);
		writeString(ptr, tempBuffer);
		ridesStructArray[i].score_d = (short) atoi(tempBuffer);
		writeString(ptr, tempBuffer);
		ridesStructArray[i].tip = (short) atof(tempBuffer);
		ridesStructArray[i].comment = loadString(ptr); // e se for null?????????????????

		temp = &(ridesStructArray[i]);
		// check if city is not already in hash table
		if ((array = g_hash_table_lookup(cityTable, city)) == NULL) {
			// if not, insert
			array = g_ptr_array_sized_new((1 << 17) + (1 << 15));
			g_ptr_array_add(array, temp);
			g_hash_table_insert(cityTable, city, array);
		} else {
			// if yes, append to all the other data
			g_ptr_array_add(array, temp);
			// printf("Adding to %s a ride in %s\n", city, temp->city);
		}

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

void freeRidesData(DATA data) {
	RidesData *dataStruct = data;
	RidesStruct **ridesData = dataStruct->ridesArray;
	GHashTable *table = dataStruct->cityTable;
	g_hash_table_destroy(table);
	int i, j;
	RidesStruct *segment, block;
	for (i = 0; i < RIDES_ARR_SIZE; i++) {
		segment = ridesData[i];
		for (j = 0; j < SIZE; j++) {
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

//devolve a struct(dados) associada à ride número i
RidesStruct * getRideByID(DATA data, guint ID) {
	ID -= 1; // para o primeiro passar a ser 0
	int i = ID / RIDES_ARR_SIZE;
	RidesStruct **primaryArray = data,
	* secondaryArray = primaryArray[i],
	* result = &(secondaryArray[ID - SIZE*i]);
	return result;
}

RidesStruct * getRidePtrByID(DATA data, guint ID) {
	ID -= 1; // para o primeiro passar a ser 0
	int i = ID / RIDES_ARR_SIZE;
	RidesStruct **primaryArray = data,
	* secondaryArray = primaryArray[i],
	* result = &(secondaryArray[ID - SIZE*i]);
	return result;
}

void freeArray(void *data) {
	GPtrArray *array = (GPtrArray *)data;
	g_ptr_array_free(array, TRUE);
}

gint compareRidesByDate (gconstpointer a, gconstpointer b) {
	gint res;
	char *dateA = (*(RidesStruct **)a)->date, *dateB = (*(RidesStruct **)b)->date;
	//DD/MM/YYYY
	if ((res = strncmp(dateA + 6, dateB + 6, 4)) != 0) {
		return res;
	} else if ((res = strncmp(dateA + 3,dateB + 3, 2)) != 0) {
		return res;
	} else {
		return (strncmp(dateA,dateB, 2));
	}
}

CityRides * getRidesByCity(RidesData *data, char *city) {
	CityRides *resultRides = malloc(sizeof(CityRides));
	resultRides->array = g_hash_table_lookup(data->cityTable, city);
	return resultRides;
}

guint getNumberOfCityRides(CityRides *rides) {
	return rides->array->len;
}

RidesStruct *getCityRidesByID(CityRides *rides, guint ID) {
	RidesStruct *ride = (RidesStruct *)g_ptr_array_index(rides->array, (int)ID),
	*result = malloc(sizeof(RidesStruct));
	// FALTA COPIAR AS STRINGS, FAZER COPYRIDE()
	memcpy(result, ride, sizeof(RidesStruct));
	return result;
}
