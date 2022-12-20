#!/bin/bash

make -j$(nproc) 1>/dev/null && ./programa-principal Dataset_Fase1 teste1.txt
