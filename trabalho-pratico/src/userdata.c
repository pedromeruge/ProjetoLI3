#include "userdata.h"

#define LINES 100000

DATA getUserData(FILE *ptr)
{
	while (fgetc(ptr) != '\n')
		; // avançar a primeira linha
	int line;
	char *username, tempchr;
	Userstruct *userstruct;
	GHashTable *table = g_hash_table_new_full(g_str_hash, g_str_equal, free, freeTableData); /////// NOT FREE
	for (line = 0; line < LINES; line++)
	{
		userstruct = malloc(sizeof(Userstruct));
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

void freeTableData(void *userData)
{
	Userstruct *data = userData;
	free(data->name);
	free(data->birthdate);
	free(data->accountCreation);
	free(data);
}

void freeUserData(DATA userdata)
{
	UserData * data = (UserData *) userdata;
	g_hash_table_destroy(data->table);
	free(data);
}

