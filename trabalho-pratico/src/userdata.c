#include "userdata.h"

#define LINES 100000
#define USER_STR_BUFF 32

#define N_OF_FIELDS 7

#define dupe_str(str) return strndup(str, USER_STR_BUFF)

#define USER_IS_VALID(user) (user->username != NULL)

#define STR_BUFF_SIZE 64

struct UserStruct
{
	char *username;
	char *name;
	unsigned char gender;
	Date birthdate,
		 accountCreation;
	// unsigned char payMethod;
	unsigned char status;
	int score;
	short int total[3];
	short int distance[3];
	float tips;
	Date mostRecRideDate;
};

struct UserData
{
	GHashTable * table;
	GPtrArray * userDistanceArray;
};

void freeTableData(void *userData)
{
	UserStruct *data = userData;
	free(data->name);
	//free(data->birthdate);
	//free(data->accountCreation);
	free(data);

}

void dumpUser(UserStruct *user);

UserData *getUserData(FILE *ptr, char *buffer)
{
	UserData *data = malloc(sizeof(UserData));
	GPtrArray *array = g_ptr_array_sized_new(50000);
	data->userDistanceArray = array;
	while (fgetc(ptr) != '\n'); // avançar a primeira linha
	char *username;
	UserStruct *userstruct;
	GHashTable *table = g_hash_table_new_full(g_str_hash, g_str_equal, free, freeTableData); /////// NOT FREE
	
	parse_format format;

	parse_func_struct format_array[N_OF_FIELDS] = {
		{ p_getString, offsetof(UserStruct, username), 1, },
		{ p_getString, offsetof(UserStruct, name), 1, },
		{ p_getGender, offsetof(UserStruct, gender), 0, },
		{ p_getDate, offsetof(UserStruct, birthdate), 0, },
		{ p_getDate, offsetof(UserStruct, accountCreation), 0, },
		{ p_getPayMethod, 0, 0, }, // este offset nao interessa
		// // o parametro é inútil mas temos de fazer check para ver se está vazio, a função nunca escreve nada na struct do user
		{ p_getAccountStatus, offsetof(UserStruct, status), 0, },
	};

	// que confusao nao sei fazer nomes
	format.format_array = format_array;
	format.len = N_OF_FIELDS;

	int res = 1;
	int num = 0;
	int invalid = 0;
	int bp = 0, sp = 0;

	while (1) //(res != -1) // acaba por ser inutil por causa da condiçao do break
	{
		userstruct = malloc(sizeof(UserStruct));
		if ((res = parse_with_format(ptr, userstruct, &format, &bp, &sp, buffer)) == 1)
		{
			// dumpUser(userstruct);

			if (userstruct->status == 1) g_ptr_array_add(array, userstruct); // só adiciona se for ativo
			memset((char *)userstruct + offsetof(UserStruct, score), 0, (sizeof(float) + (2 * sizeof(int)) + (6 * sizeof(short int))));
			username = userstruct->username;
			if (g_hash_table_insert(table, username, userstruct) == FALSE)
			{
				fprintf(stderr, "Username already existed\n");
				// printf("Buffer: %s\n %d %d\n", buffer, bp, sp);
				exit(5);
			}
		} else {
			free(userstruct);
			if (res == -1) break;
			//else if == 0
			invalid++;
		}

		// printf("\"username:%s name:%s gender:%c birthdate:%s accCreation:%s payMethod:%c(%d) status:%c(%d)\"\n",
		// username, userstruct->name, userstruct->gender, userstruct->birthdate, userstruct->accountCreation, userstruct->payMethod, userstruct->payMethod, userstruct->status, userstruct->status);
		
		num++;
	}
	data->table = table;

	printf("Total number of users: %d\nNumber of invalid users: %d\n", num, invalid);

	return data;
}

inline UserStruct *getUserPtrByUsername(const UserData *data, const char *name)
{
	GHashTable *table = data->table;
	return (UserStruct *)(g_hash_table_lookup(table, name));
}

inline char *getUserName(const UserStruct *data)
{
	dupe_str(data->name);
	// return strndup(data->name, USER_STR_BUFF);
}

inline char *getUserUsername(const UserStruct *data)
{
	dupe_str(data->username);
	// return strndup(data->username, USER_STR_BUFF);
}

inline unsigned char getUserGender(const UserStruct *data)
{
	return (data->gender);
}

inline Date getUserBirthdate(const UserStruct *data)
{
	return data->birthdate;
}

inline Date getUserAccCreation(const UserStruct *data)
{
	return data->accountCreation;
}

unsigned char getUserStatus(const UserStruct *data)
{
	return (data->status);
}

void freeUserData(UserData *userData)
{
    g_hash_table_destroy(userData->table);
    g_ptr_array_free(userData->userDistanceArray, TRUE);
    free(userData);
}

int userIsValid(const UserStruct *user) {
	return (user != NULL && USER_IS_VALID(user));
}

void add_user_info (const UserData* data, const DriverData* driverdata, const char* name, int driver, int distance, int score, float tip, Date date) {
    int carClass = getDriverCar(getDriverPtrByID(driverdata, driver));
	UserStruct* user = g_hash_table_lookup(data->table, name);
	(user->tips) += tip;
    (user->total) [carClass] += 1;
	(user->score) += score;
    (user->distance) [carClass] += distance;
	if(user->mostRecRideDate == 0 || (compDates(user->mostRecRideDate, date) < 0)) {
		user->mostRecRideDate = date;
	}
}

inline int getUserNumberOfRides (const UserStruct* user) {
	return (user->total[0] + user->total[1] + user->total[2]);
}

inline double getAvgUserRating (const UserStruct* user) {
	return ((double)user->score / (double)(user->total[0] + user->total[1] + user->total[2]));
}

inline double getUserTotalSpent (const UserStruct* user) {
	return ((double)(user->total[0] * 3.25 + user->total[1] * 4 + user->total[2] * 5.2 + user->distance[0] * 0.62 + user->distance[1] * 0.79 + user->distance[2] * 0.94 + user->tips));
}

inline int getUserTotalDistance (const UserStruct* user) {
	return (user->distance[0] + user->distance[1] + user->distance[2]);
}

gint userDistComp (gconstpointer a, gconstpointer b) {
    UserStruct * user1 = *(UserStruct **) a, * user2 = *(UserStruct **) b;
    short int * distance1 = user1->distance, * distance2 = user2->distance; 
    gint result =  distance1[0] + distance1[1] + distance1[2] - distance2[0] - distance2[1] - distance2[2];
    if (result == 0 && user1->mostRecRideDate != 0) {
        result = compDates(user1->mostRecRideDate, user2->mostRecRideDate);
        if (result == 0) {
            return strcmp(user2->username, user1->username);
        } else return result;
    } else return result;
}

void sort_userarray (const UserData * data) {
	g_ptr_array_sort(data->userDistanceArray, userDistComp); 
}


char * userTopN (const UserData * data, int N) {
    int i, dist;
    char * name, * username;
    char * result = malloc(N * STR_BUFF_SIZE * sizeof(char));
    // if (result == NULL) exit (1);
    result[0] = '\0';
    GPtrArray * array = data->userDistanceArray;
    int arraylength = array->len;
    UserStruct * userInf;

	int offset = 0;

    for(i = arraylength - 1; i > (const int)(arraylength - N - 1); i--) {
        userInf = (UserStruct *)g_ptr_array_index(array, i);
        username = userInf->username;
        name = userInf->name;
        dist = userInf->distance[0] + userInf->distance[1] + userInf->distance[2];
        offset += snprintf(result + offset, STR_BUFF_SIZE, "%s;%s;%d\n", username, name, dist);
    }
    return result;
}

void dumpUser(UserStruct *user) {
	printf("%s %s %u %u/%u/%u %u/%u/%u %u\n",
	user->username, user->name, user->gender, GET_DATE_DAY(user->birthdate), GET_DATE_MONTH(user->birthdate), GET_DATE_YEAR(user->birthdate),
	GET_DATE_DAY(user->accountCreation), GET_DATE_MONTH(user->accountCreation), GET_DATE_YEAR(user->accountCreation), user->status);
	fflush(stdout);
}
