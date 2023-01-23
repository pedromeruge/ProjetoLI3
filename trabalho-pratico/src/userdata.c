#include "userdata.h"

#define LINES 100000
#define USER_STR_BUFF 32

#define N_OF_FIELDS 7

#define dupe_str(str) return strndup(str, USER_STR_BUFF)

#define USER_IS_VALID(user) (user->username != NULL)

struct UserStruct
{
	char *username;
	char *name;
	unsigned char gender;
	char *birthdate;
	char *accountCreation;
	// unsigned char payMethod;
	unsigned char status;
	int score;
	short int total[3];
	short int distance[3];
	float tips;
};

struct UserData
{
	GHashTable *table;
};

void freeTableData(void *userData)
{
	UserStruct *data = userData;
	free(data->name);
	free(data->birthdate);
	free(data->accountCreation);
	free(data);
}

UserData *getUserData(FILE *ptr)
{
	while (fgetc(ptr) != '\n'); // avançar a primeira linha
	char *username;
	UserStruct *userstruct;
	GHashTable *table = g_hash_table_new_full(g_str_hash, g_str_equal, free, freeTableData); /////// NOT FREE
	
	parse_format format;

	parse_func_struct format_array[N_OF_FIELDS] = {
		{ p_getUserName, offsetof(UserStruct, username), 1, },
		{ p_getName, offsetof(UserStruct, name), 1, },
		{ p_getGender, offsetof(UserStruct, gender), 0, },
		{ p_getDate, offsetof(UserStruct, birthdate), 1, },
		{ p_getDate, offsetof(UserStruct, accountCreation), 1, },
		{ p_getPayMethod, 0, 0, }, // este offset nao interessa
		// o parametro é inútil mas temos de fazer check para ver se está vazio, a função nunca escreve nada na struct do user
		{ p_getAccountStatus, offsetof(UserStruct, status), 0, },
	};

	// que confusao nao sei fazer nomes
	format.format_array = format_array;
	format.len = N_OF_FIELDS;

	int res = 1;
	int num = 0;
	int invalid = 0;

	while (1) //(res != -1) // acaba por ser inutil por causa da condiçao do break
	{
		userstruct = malloc(sizeof(UserStruct));
		if ((res = parse_with_format(ptr, userstruct, &format)) == 1)
		{
			memset((char *)userstruct + offsetof(UserStruct, score), 0, (sizeof(float) + sizeof(int) + (6 * sizeof(short int))));
			username = userstruct->username;
			if (g_hash_table_insert(table, username, userstruct) == FALSE)
			{
				fprintf(stderr, "Username already existed\n");
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
	UserData *data = malloc(sizeof(UserData));
	data->table = table;

	printf("Total number of users: %d\nNumber of invalid users: %d\n", num, invalid);

	return data;
}

UserStruct *getUserPtrByUsername(UserData *data, char *name)
{
	GHashTable *table = data->table;
	return (UserStruct *)(g_hash_table_lookup(table, name));
}

char *getUserName(UserStruct *data)
{
	dupe_str(data->name);
	// return strndup(data->name, USER_STR_BUFF);
}

char *getUserUsername(UserStruct *data)
{
	dupe_str(data->username);
	// return strndup(data->username, USER_STR_BUFF);
}

unsigned char getUserGender(UserStruct *data)
{
	return (data->gender);
}

char *getUserBirthdate(UserStruct *data)
{
	dupe_str(data->birthdate);
	// return strndup(data->birthdate, USER_STR_BUFF);
}

char *getUserAccCreation(UserStruct *data)
{
	dupe_str(data->accountCreation);
	// return strndup(data->accountCreation, USER_STR_BUFF);
}

// unsigned char getUserPayMethod(UserStruct *data)
// {
// 	return (data->payMethod);
// }

unsigned char getUserStatus(UserStruct *data)
{
	return (data->status);
}

void freeUserData(UserData *userData)
{
	g_hash_table_destroy(userData->table);
	free(userData);
}

int userIsValid(UserStruct *user) {
	return (user != NULL && USER_IS_VALID(user));
}

void add_user_info (UserData* data, DriverData* driverdata, char* name, int driver, int distance, int score, float tip) {
    int carClass = getDriverCar(getDriverPtrByID(driverdata, driver));
	UserStruct* user = g_hash_table_lookup(data->table, name);
	(user->tips) += tip;
    (user->total) [carClass] += 1;
	(user->score) += score;
    (user->distance) [carClass] += distance;
}

inline int getUserNumberOfRides (UserStruct* user) {
	return (user->total[0] + user->total[1] + user->total[2]);
}

inline double getAvgUserRating (UserStruct* user) {
	return ((double)user->score / (double)(user->total[0] + user->total[1] + user->total[2]));
}

inline double getUserTotalSpent (UserStruct* user) {
	return ((double)(user->total[0] * 3.25 + user->total[1] * 4 + user->total[2] * 5.2 + user->distance[0] * 0.62 + user->distance[1] * 0.79 + user->distance[2] * 0.94 + user->tips));
}

inline int getUserTotalDistance (UserStruct* user) {
	return (user->distance[0] + user->distance[1] + user->distance[2]);
}
