#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
#include "query_requests.h"
#include "files.h"

#define TEST_OUT_PATH "testes_output.txt"

int main (int argc, char **argv) {
    clock_t cpu_start, cpu_end;
	double cpu_time_used;
	struct timespec start, finish, delta;

	// se não for dado o ficheiro e as querries no terminal - fase 2
    if (argc < 2) {
        fprintf(stderr, "Modo interativo por implementar!\n");
		exit(1);
	}
	//caso seja facultado o ficheiro e as querries no terminal - fase 1
    FILE ** files = open_cmdfiles(argv); // array com pointers para os ficheiros {users,drivers,rides,querries}
	FILE *test_output = fopen(TEST_OUT_PATH, "w");

	cpu_start = clock();
	clock_gettime(CLOCK_REALTIME, &start);

	DATA users = getUserData(files[0]);
	
	DATA drivers = getDriverData(files[1]);

	DATA rides = getRidesData(files[2]);

	cpu_end = clock();
	clock_gettime(CLOCK_REALTIME, &finish);
	sub_timespec(start, finish, &delta);
	cpu_time_used = ((double) (cpu_end - cpu_start)) / CLOCKS_PER_SEC;
	printf("Loading data:\nCPU time:%g\n", cpu_time_used);
	printf("Wall clock time:%d.%.9ld\n\n", (int)delta.tv_sec, delta.tv_nsec);
	char buffer[128];
	snprintf(buffer, 128, "Loading Data\nCPU time:%g\nWall clock time:%d.%.9ld\n------------------------\n", cpu_time_used, (int)delta.tv_sec, delta.tv_nsec);
	fputs(buffer, test_output);
	
	int ret = queryRequests(files[3], users, drivers, rides, test_output);
    if (ret) {
    	fprintf(stderr, "Error reading query requests");
        return 2;
    }

	fclose(files[0]);
	fclose(files[1]);
	fclose(files[2]);
	fclose(files[3]);
	fclose(test_output);

	cpu_start = clock();
	clock_gettime(CLOCK_REALTIME, &start);

	freeUserData(users);
	freeDriverData(drivers);
	freeRidesData(rides);
	free(files);

	cpu_end = clock();
	clock_gettime(CLOCK_REALTIME, &finish);
	sub_timespec(start, finish, &delta);
	cpu_time_used = ((double) (cpu_end - cpu_start)) / CLOCKS_PER_SEC;
	printf("Freeing data:\nCPU time:%g\n", cpu_time_used);
	printf("Wall clock time:%d.%.9ld\n", (int)delta.tv_sec, delta.tv_nsec);

	return 0;
}
