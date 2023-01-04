#!/bin/bash

cd ..
echo "Compiling"
make clean &> /dev/null
make -j$(nproc) PROFILING_OPTS=-pg &> /dev/null && make debug -j$(nproc) PROFILING_OPTS=-p &> /dev/null
# ia fazelos em paralelo mas deve dar problemas com o output dos ficheiros
echo "Running"
rm Resultados/* &> /dev/null
./programa-principal Dataset_Fase1 teste1.txt
gprof -b programa-principal gmon.out > perf_principal_s.txt
rm Resultados/* &> /dev/null
./programa-debug Dataset_Fase1 teste1.txt
gprof -b programa-debug gmon.out > perf_debug_s.txt
rm Resultados/* &> /dev/null
./programa-principal Dataset_Fase2/data-regular-errors exemplos_de_queries/fase2/regular-errors-outputs/input.txt
gprof -b programa-principal gmon.out > perf_principal_s_e.txt
rm Resultados/* &> /dev/null
./programa-debug Dataset_Fase2/data-regular-errors exemplos_de_queries/fase2/regular-errors-outputs/input.txt
gprof -b programa-debug gmon.out > perf_debug_s_e.txt
rm Resultados/* &> /dev/null
./programa-principal Dataset_Fase2/data-large exemplos_de_queries/fase2/large-outputs/input.txt
gprof -b programa-principal gmon.out > perf_principal_l.txt
rm Resultados/* &> /dev/null
./programa-debug Dataset_Fase2/data-large exemplos_de_queries/fase2/large-outputs/input.txt
gprof -b programa-debug gmon.out > perf_debug_l.txt
rm Resultados/* &> /dev/null
./programa-principal Dataset_Fase2/data-large-errors exemplos_de_queries/fase2/large-errors-outputs/input.txt
gprof -b programa-principal gmon.out > perf_principal_l_e.txt
rm Resultados/* &> /dev/null
./programa-debug Dataset_Fase2/data-large-errors exemplos_de_queries/fase2/large-errors-outputs/input.txt
gprof -b programa-debug gmon.out > perf_debug_l_e.txt
# que coisa feia
rm gmon*