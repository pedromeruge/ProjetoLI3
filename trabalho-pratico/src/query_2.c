#include "query_2.h"

// TODO: Fazer parse de um array com ordem dos users e outro para rides em simultâneo 
//       função listN (void *array, struct *, comparefunc1,comparefunc2,comparefunc3 )

//concatena n linhas com os resultados, para dar return da query_2

// static char * topN(void * resultArray, int N, void * dataArray, outputfunc1,outputfunc2,outputfunc3) {
//     return NULL
// }

char * query_2 (char * inputStr[], UserData *userData, DriverData * driverData, RidesData *ridesData) {
	int num = atoi(inputStr[0]);
	if (num == 0) return NULL;

    const ridesByDriverGlobal * driverRatingArray = getRidesByDriverGlobalSorted (ridesData);
    
    char * result = topNGlobal(driverRatingArray,num, driverData);

    free((ridesByDriverGlobal *) driverRatingArray);

    // NOTA: query 3 é indentica a função query_2 mas usa outros valores e outra compare func. Fazer um ptr_array com toda a informação e ordená-lo segundo a função dada talvez??
    // NOTA: a maior parte das funções de queries resume-se a "n primeiros" e "valores médios" apenas variando os parâmetros de comparação e o abrangimento dos dados de input. Estas funções mais básicas devem ser capazes de receber diferentes categorias (mais ou menos restritas) de dados e ordená-las com base nos parâmetros dados
    return result;
}
