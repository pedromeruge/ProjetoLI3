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
	char buffer[128];
	char *main_buffer = malloc(SIZE_OF_READ_BUFFER);

	if (argc == 2) { // testar o tempo de load, mais nada
		FILE ** files = open_cmdfiles(argc,argv);
		FILE *output = fopen("testes_final_output.txt", "a");

		cpu_start = clock();
		clock_gettime(CLOCK_REALTIME, &start);

		UserData * users = getUserData(files[0], main_buffer);
	
		DriverData* drivers = getDriverData(files[1], main_buffer);

		RidesData * rides = getRidesData(files[2], users, drivers, main_buffer);

		cpu_end = clock();
		clock_gettime(CLOCK_REALTIME, &finish);
		sub_timespec(start, finish, &delta);
		cpu_time_used = ((double) (cpu_end - cpu_start)) / CLOCKS_PER_SEC;
		printf("Loading data:\n%lf/%d.%.9ld\n", cpu_time_used, (int)delta.tv_sec, delta.tv_nsec);
		fprintf(output, "%lf %d.%.9ld\n", cpu_time_used, (int)delta.tv_sec, delta.tv_nsec);

		fclose(files[0]);
		fclose(files[1]);
		fclose(files[2]);

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
		printf("Freeing data:\n%lf/%d.%.9ld\n", cpu_time_used, (int)delta.tv_sec, delta.tv_nsec);
		fprintf(output, "%lf %d.%.9ld\n", cpu_time_used, (int)delta.tv_sec, delta.tv_nsec);

		fclose(output);

		return 0;
	}

	if (argc < 3) {
		fprintf(stderr, "Input para testes: dataset, batch file, diretoria com os outputs para serem comparados\nOu so o path para dataset para testar tempos de load e free");
		return 1;
	}
	//caso seja facultado o ficheiro e as querries no terminal - fase 1
    FILE ** files = open_cmdfiles(argc,argv); // array com pointers para os ficheiros {users,drivers,rides,querries}
	FILE *test_output = fopen(TEST_OUT_PATH, "w");

	cpu_start = clock();
	clock_gettime(CLOCK_REALTIME, &start);

	UserData * users = getUserData(files[0], main_buffer);
	
	DriverData * drivers = getDriverData(files[1], main_buffer);

	RidesData * rides = getRidesData(files[2], users, drivers, main_buffer);

	cpu_end = clock();
	clock_gettime(CLOCK_REALTIME, &finish);
	sub_timespec(start, finish, &delta);
	cpu_time_used = ((double) (cpu_end - cpu_start)) / CLOCKS_PER_SEC;
	printf("Loading data:\nCPU time:%g\n", cpu_time_used);
	printf("Wall clock time:%d.%.9ld\n\n", (int)delta.tv_sec, delta.tv_nsec);
	snprintf(buffer, 128, "Loading Data\nCPU time:%lf\nWall clock time:%d.%.9ld\n------------------------\n", cpu_time_used, (int)delta.tv_sec, delta.tv_nsec);
	fputs(buffer, test_output);
	
	int ret = fileRequests(files[3], users, drivers, rides, test_output, argv[3]);
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
	free(main_buffer);

	cpu_end = clock();
	clock_gettime(CLOCK_REALTIME, &finish);
	sub_timespec(start, finish, &delta);
	cpu_time_used = ((double) (cpu_end - cpu_start)) / CLOCKS_PER_SEC;
	printf("Freeing data:\nCPU time:%lf\n", cpu_time_used);
	printf("Wall clock time:%d.%.9ld\n", (int)delta.tv_sec, delta.tv_nsec);

	return 0;
}
