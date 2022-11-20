#include "userdata.h"

#define LINES 100000
#define USER_STR_BUFF 32

struct UserStruct
{
	// char *username;
	char *name;
	unsigned char gender;
	char *birthdate;
	char *accountCreation;
	unsigned char payMethod;
	unsigned char status;
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
	while (fgetc(ptr) != '\n')
		; // avançar a primeira linha
	int line;
	char *username, tempchr;
	UserStruct *userstruct;
	GHashTable *table = g_hash_table_new_full(g_str_hash, g_str_equal, free, freeTableData); /////// NOT FREE
	for (line = 0; line < LINES; line++)
	{
		userstruct = malloc(sizeof(UserStruct));
		username = loadString(ptr);
		userstruct->name = loadString(ptr);
		userstruct->gender = fgetc(ptr);
		fseek(ptr, 1, SEEK_CUR);
		userstruct->birthdate = loadString(ptr);
		userstruct->accountCreation = loadString(ptr);
		userstruct->payMethod = getPayMethod(ptr);
		userstruct->status = getAccountStatus(ptr);

		// avaçar até proxima linha
		while ((tempchr = fgetc(ptr)) != '\n')
			; // && (tempchr != -1));

		// printf("\"username:%s name:%s gender:%c birthdate:%s accCreation:%s payMethod:%c status:%c\"\n",
		// username, userstruct->name, userstruct->gender, userstruct->birthdate, userstruct->accountCreation, userstruct->payMethod, userstruct->status);

		// if (username == NULL) ???
		if (g_hash_table_insert(table, username, userstruct) == FALSE)
		{
			fprintf(stderr, "Username already existed\n");
			exit(5);
		}
	}
	UserData *data = malloc(sizeof(UserData));
	data->table = table;
	return data;
}

UserStruct *getUserPtrByUsername(UserData *data, char *name)
{
	GHashTable *table = data->table;
	return (UserStruct *)(g_hash_table_lookup(table, name));
}

char *getUserName(UserStruct *data)
{
	return strndup(data->name, USER_STR_BUFF);
}

unsigned char getUserGender(UserStruct *data)
{
	return (data->gender);
}

char *getUserBirthdate(UserStruct *data)
{
	return strndup(data->birthdate, USER_STR_BUFF);
}

char *getUserAccCreation(UserStruct *data)
{
	return strndup(data->accountCreation, USER_STR_BUFF);
}

unsigned char getUserPayMethod(UserStruct *data)
{
	return (data->payMethod);
}

unsigned char getUserStatus(UserStruct *data)
{
	return (data->status);
}

void freeUserData(UserData *userData)
{
	g_hash_table_destroy(userData->table);
	free(userData);
}
