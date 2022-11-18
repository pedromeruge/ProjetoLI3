#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
#include "query_requests.h"
#include "files.h"

int main (int argc, char **argv) {
    // se não for dado o ficheiro e as querries no terminal - fase 2
    if (argc < 2) {
        fprintf(stderr, "Modo interativo por implementar!\n");
		exit(1);
	}

	//caso seja facultado o ficheiro e as querries no terminal - fase 1
    FILE ** files = open_cmdfiles(argv); // array com pointers para os ficheiros {users,drivers,rides,querries}

	DATA users = getUserData(files[0]);
	
	DriverData * drivers = getDriverData(files[1]);

	RidesData * rides = getRidesData(files[2]);
	
	int ret = queryRequests(files[3], users, drivers, rides);
    if (ret) {
    	fprintf(stderr, "Error reading query requests, return value: %d\n",ret);
        return 2;
    }

	fclose(files[0]);
	fclose(files[1]);
	fclose(files[2]);
	fclose(files[3]);

	freeUserData(users);
	//freeDriverData(drivers);
	freeRidesData(rides);
	free(files);

	return 0;
}
