#!/bin/bash

make -j$(nproc) testes 1>/dev/null && ./programa-testes Dataset_Fase1 exemplos_de_queries/tests_1/input.txt