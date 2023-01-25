#include <stdlib.h>
#include <stdio.h>
#include "query_requests.h"
#include "files.h"
#include "query_dispatch_table.h"
#include <string.h>
#include "userdata.h"
#include "driverdata.h"
#include "ridesData.h"
#include <unistd.h>

#define LINE_SIZE 128
#define PATH_SIZE 128
#define N_OF_REPETITIONS 10
#define MAX_QUERY_INPUTS 3

typedef void q_test_func (UserData* userData, DriverData *driverData, RidesData *ridesData);

char * query_not_implemented(char *inputStr[], UserData *userData, DriverData * DriverData, RidesData *ridesData) {
	fprintf(stderr, "ERROR: querry not implemented\n");
	return NULL;
}

void q_test_undefined(UserData* userData, DriverData *driverData, RidesData *ridesData) {
	fprintf(stderr, "Querry not implemented or time does not directly depend on input (but only on data)\n");
}

void test_q_2 (UserData* userData, DriverData *driverData, RidesData *ridesData) {
	struct timespec start, finish, delta;
	int sec = 0, N = 50, total;
	char str[16], *res, buff[16];
	char *strInput[3];
	int NUMBER_OF_DRIVERS = 10000; // mudar isto!!!!

	//aproximação por linear curve fitting

	// codigo emprestado de https://www.codewithc.com/c-program-for-linear-exponential-curve-fitting/
	// https://blog.mbedded.ninja/mathematics/curve-fitting/linear-curve-fitting/
	
	long double y, sumy=0,sumxy=0, m, b, firsty = 0;
	long int sumx=0, sumx2=0, firstx = 0;

	//warmup cache ?????
	snprintf(str, 16, "%d", 1);
	strInput[0] = str;
	res = query_2(strInput, userData, driverData, ridesData);
	free(res);

	for (total = 1; sec < 10 && N < NUMBER_OF_DRIVERS; total++) {
		clock_gettime(CLOCK_REALTIME, &start);
		snprintf(str, 16, "%d", N);
		res = query_2(strInput, userData, driverData, ridesData);
		free(res);
		clock_gettime(CLOCK_REALTIME, &finish);
		sub_timespec(start, finish, &delta);
		sec = (int)delta.tv_sec;
		// printf("%d Took %d.%.9ld seconds for N=%d\n", i, (int)delta.tv_sec, delta.tv_nsec, N);
		snprintf(buff, 16, "%d.%ld", (int)delta.tv_sec, delta.tv_nsec);
		sscanf(buff, "%Lf", &y);
		printf("x:%d y:%Lf\n", N, y);
		sumx += N;
		sumy += y;
		sumx2 += N*N;
		sumxy += N*y;
		if (firsty == 0 && firstx == 0) {
			firstx = N; firsty = y;
		}
		
		N *= 2;
	}

	if (sec != 10) {
		// for(i = 0; i < total; i++) {
		// 	sumx=sumx +x[i];
		// 	sumx2=sumx2 +x[i]*x[i];
		// 	sumy=sumy +y[i];
		// 	sumxy=sumxy +x[i]*y[i];
		// }
		// printf("%ld %ld %lf %lf \n", sumx, sumx2, sumy, sumxy);
    	m=((total*sumxy-sumx*sumy)*1.0/(total*sumx2-sumx*sumx)*1.0);
		b=((sumx2*sumy -sumx*sumxy)*1.0/(total*sumx2-sumx*sumx)*1.0);
		// sumxy=sumxy/total;
		// sumx=sumx/total;
		// sumy=sumy/total;
		// sumx2=sumx2/total;
		// m=(sumxy-sumx*sumy)/(sumx2-sumx*sumx);
		
		// printf("\n\nThe line is Y=%Lf X + (%Lf)\n",m,b);
		printf("Estimated N for 10 seconds of time taken:%Lf\n", (10.0 - b) / m);
		printf("Rougher aproximation: %Lf\n", (10*((N/2) - firstx)) / (y - firsty));
	}


	
	//printf("Wall clock time:%d.%.9ld\n\n", (int)delta.tv_sec, delta.tv_nsec);
}

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

// nao contamos o ultimo \n
int compareResult(char *resultStr, char *resultPath) {
	int i;
	int chr = EOF;
	int ret = 0; // se retornar isto, deu valores e devia dar valores
	FILE *fpout = fopen(resultPath, "r");
	if (!fpout) {
		perror("Unable to open/create output file");
    	return -1;
	}
	if (resultStr == NULL) {
		chr = fgetc(fpout);
		if (chr == EOF) ret = 0; // deu NULL e devia dar NULL (ou não conseguiu ler o ficheiro -> fgetc tem o mesmo return)
		else ret = 2; // deu NULL, e devia dar valores;
	} else {
		fseek(fpout,-1,SEEK_CUR); // retrocede um caractér (movido pelo fgetc) // melhor que rewind ou pior??
		for (i = 0; resultStr[i] != '\0' && (chr = fgetc(fpout)) != EOF && (!ret); i++) { // fgetc individual é melhor que getline ??
			// printf("|%d %d %c %c\n", chr, (int) resultStr[i], (char)chr, resultStr[i]);
			if ((char)chr != resultStr[i]) {
				fprintf(stderr, "Error on character [%d]", i);
				ret = 3; // deu valores diferentes dos supostos
			}
		}
	}
	fclose(fpout);
	return ret;
}

int writeResultsTests (int commandN, char * strResult, const char *command_path) {
	char resultPath[PATH_SIZE];
	snprintf(resultPath, PATH_SIZE, "%s/command%d_output.txt", command_path, commandN);
	
	return compareResult(strResult, resultPath);
}

int fileRequests (FILE * fp, UserData *userData, DriverData *driverData, RidesData *ridesData, FILE *test_output, char *command_path) {
	clock_t cpu_start, cpu_end;
	double cpu_time_used;
	struct timespec start, finish, delta;

	query_func * queryList[9] = {query_1, query_2, query_not_implemented, query_4, query_5, query_6, query_7, query_not_implemented, query_9};
    char * strBuffer = malloc(sizeof(char)*LINE_SIZE); // buffer de cada linha lida
    char * querryResult = NULL; // pointer para a string resultante de cada querry
    char * tempsegstr[MAX_QUERY_INPUTS + 1]; // array para atribuir o segmento correto do input
    char * strHolder, *temp;
    ssize_t read; size_t len = LINE_SIZE; // para o getline
    int i,j, commandN = 1, writeRet;
	int test;
	char outBuffer[128];

	double cpu_time_acc;
	long double wall_clock_time_acc, tempAcc;
	// q_test_func *test_funcs[9] = {q_test_undefined, test_q_2, q_test_undefined, q_test_undefined, q_test_undefined, q_test_undefined, q_test_undefined, q_test_undefined, q_test_undefined};

	int total[9] = {0};
	double time[9] = {0};
	int query;

    // lê linhas individualmente até chegar ao fim do ficheiro
	for (i=0; (read = getline(&strBuffer, &len, fp) != -1); i++, commandN++) {
		// a primeira vez corre fora do loop para dar output para o ficheiro, as outras já não fazem output


		strBuffer[strcspn(strBuffer, "\n")] = 0; // para remover o newline
		fputs(strBuffer, test_output);
		fputc('\n', test_output);

		temp = strBuffer;

		tempsegstr[0] = NULL;
		tempsegstr[1] = NULL;
		tempsegstr[2] = NULL;
		tempsegstr[3] = NULL;

		temp = strBuffer;
		for (j = 0; j < 4 && (strHolder = strsep(&strBuffer," ")); j++) { // j<4 por segurança
			tempsegstr[j] = strHolder;
		}
		strBuffer = temp;

		for (j = 0; j <= MAX_QUERY_INPUTS && (strHolder = strsep(&strBuffer, " ")); j++)
        {
            tempsegstr[j] = strHolder;
        }
        strBuffer = temp;

		query = (*tempsegstr[0]) - 49; // -48 para dar o numero correto, -1 para a query 1 dar no lugar 0
        //print de debug para os inputs de cada query
        fprintf(stderr, "command (%d), query |%d| input segments:", commandN, query + 1);

        for (j = 1;j <= MAX_QUERY_INPUTS && tempsegstr[j]; j++) {
            fprintf(stderr," <%.16s>",tempsegstr[j]);
		}
		fputc('\n', stderr);

		cpu_start = clock();
		clock_gettime(CLOCK_REALTIME, &start);
		
		querryResult = queryList[query] (tempsegstr + 1,userData,driverData,ridesData); 
		
		cpu_end = clock();
		clock_gettime(CLOCK_REALTIME, &finish);
		sub_timespec(start, finish, &delta);
		cpu_time_used = ((double) (cpu_end - cpu_start)) / CLOCKS_PER_SEC;

		// test output
		writeRet = writeResultsTests(commandN, querryResult, command_path);
		if (writeRet == 1) {
			printf("Correct answer (querry yielded no result) || OR || Error reading file:exemplos_de_queries/tests_1/command%d_output.txt\n",commandN);
			return 3;
		}
		
		//return 2 : deu NULL e devia dar valores; return 3: deu valores diferentes
		if (writeRet == 2 || writeRet == 3) {
			fprintf(stderr, "-->ERROR: Results differ\nExpected:%s\nGot:'%s'\nError file:..../command%d_output.txt\n\n", "see file :)", querryResult, commandN);
			fputs("Error\n", test_output);
			free(querryResult); // free do buffer de output

		} else {
			printf("Correct answer, continuing tests\n");

			cpu_time_acc = cpu_time_used;
			snprintf(outBuffer, 128, "%d.%.9ld", (int)delta.tv_sec, delta.tv_nsec);
			sscanf(outBuffer, "%Lf", &wall_clock_time_acc);
			// printf("buffer value: %Lf", wall_clock_time_acc);
			snprintf(outBuffer, 128, "CPU time:%lf Wall clock time:%d.%.9ld\n", cpu_time_used, (int)delta.tv_sec, delta.tv_nsec);
			fputs(outBuffer, test_output);
			free(querryResult); // free do buffer de output
			// isto afinal é meio inútil, por agora fica comentado
			// printf("Testing upper bounds, might take a while\n");
			// test_funcs[(*tempsegstr[0]) - 49](userData, driverData, ridesData);
			for (test = 1; test < N_OF_REPETITIONS; test++) {
				cpu_start = clock();
				clock_gettime(CLOCK_REALTIME, &start);
				
				querryResult = queryList[query] (tempsegstr + 1,userData,driverData,ridesData); // -48 para dar o numero correto, -1 para a query 1 dar no lugar 0
				
				cpu_end = clock();
				clock_gettime(CLOCK_REALTIME, &finish);
				sub_timespec(start, finish, &delta);
				cpu_time_used = ((double) (cpu_end - cpu_start)) / CLOCKS_PER_SEC;

				cpu_time_acc += cpu_time_used;
				snprintf(outBuffer, 128, "%d.%.9ld", (int)delta.tv_sec, delta.tv_nsec);
				sscanf(outBuffer, "%Lf", &tempAcc);
				wall_clock_time_acc += tempAcc;
				
				// snprintf(outBuffer, 128, "buffer value: %Lf", wall_clock_time_acc);
				// fputs(outBuffer, test_output);

				free(querryResult); // free do buffer de output
				snprintf(outBuffer, 128, "CPU time:%g Wall clock time:%d.%.9ld\n", cpu_time_used, (int)delta.tv_sec, delta.tv_nsec);
				fputs(outBuffer, test_output);
			}
			snprintf(outBuffer, 128, "Average: CPU:%lf Wall clock:%Lf\n", cpu_time_acc / test, wall_clock_time_acc / test);
			fputs(outBuffer, test_output);
			total[query] += 1;
			time[query] += cpu_time_acc / test;
		}
		
		len = LINE_SIZE; // após um getline, len é alterado para o tamanho da linha; tem de ser reset, a próxima linha pode ter len maior
    }
	FILE *final_output = fopen("testes_final_output.txt", "w");
	for (i = 0; i < 9; i++) {
		if (total[i] == 0) fprintf(final_output, "%d not tested\n", i+1);
		else fprintf(final_output, "%d %lf\n", i + 1, time[i] / total[i]);
	}
    free (strBuffer); // free do buffer de input
    return 0;
}
