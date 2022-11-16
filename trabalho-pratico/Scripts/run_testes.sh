#!/bin/bash

make -j$(nproc) testes 1>/dev/null && ./programa-testes Dataset_Fase1 teste.txt
