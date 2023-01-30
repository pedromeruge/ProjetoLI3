#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
#include "query_requests.h"
#include "files.h"

int main(int argc, char **argv)
{
	// caso seja facultado o ficheiro e as querries no terminal - fase 1
	FILE **files = open_cmdfiles(argc, argv); // array com pointers para os ficheiros {users,drivers,rides,queries} // o último valor do array é NULL no modo interativo

	char *main_buffer = malloc(SIZE_OF_READ_BUFFER);

	UserData *users = getUserData(files[0], main_buffer);

	DriverData *drivers = getDriverData(files[1], main_buffer);

	// RidesData *rides = getRidesData(files[2], users, drivers);

	// printf("Finished loading\n");

	// int ret;
	// if (files[3] == NULL)
	// 	ret = interactRequests(users, drivers, rides);
	// else
	// 	ret = batchRequests(files[3], users, drivers, rides);

	// if (ret)
	// {
	// 	fprintf(stderr, "Error reading query requests, return value: %d\n", ret);
	// 	return 2;
	// }

	// printf("\nFinished queries\n");

	fclose(files[0]);
	fclose(files[1]);
	fclose(files[2]);
	fclose(files[3]);

	freeUserData(users);
	freeDriverData(drivers);
	// freeRidesData(rides);
	free(files);
	free(main_buffer);

	// printf("Finished freeing\n");

	return 0;
}
