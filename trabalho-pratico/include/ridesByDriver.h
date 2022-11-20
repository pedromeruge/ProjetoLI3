#ifndef RIDESBYDRIVER_H
#define RIDESBYDRIVER_H

#include "ridesData.h"
#include "driverdata.h"

//informações de todas a rides para um user
typedef struct driverRatingInfo {
    void * ratingChart; // void * porque vai guardar um array de ratings de 1 a 5 e distância viajada, e mais tarde é convertido para um valor médio desses ratings
    float moneyData; // inicialmente guarda número de tips, depois guarda o total_auferido
    char * mostRecRideDate; // ride mais decente
    short int ridesNumber; // inicialmente guarda cartype (porque não é utilizado), depois guardar número de rides
    short int driverNumber; // talvez meter em int o valor, ocupa menos espaço com char?
} driverRatingInfo;

//devolve um pointer array com cada nodo driverRatingInfo
GPtrArray * getRidesByDriver (DriverData *, RidesData *);

// int isRiderInactive (driverRatingInfo *) -> é preciso implementar?
// para averiguar se um rider está inativo/não existe nas rides ver se: (avgRating = *(double *) struct->ratingChart) == 0;
#endif
