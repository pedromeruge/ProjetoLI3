#ifndef FILES_H
#define FILES_H

#define FILEPATH_SIZE 128
#define RESULT_PATH "Resultados/command%d_output.txt"
#define N_OF_INPUT_FILES 3

FILE ** open_cmdfiles(int argc, char ** argv); // dados os argumentos de main, abre os ficheiros recebidos como path, no modo batch ou interativo
int writeResults(int commandN, char *strResult);

#endif
