#!/bin/bash

make -j$(nproc) testes_debug 1>/dev/null && \
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes\
	--track-origins=yes --suppressions=/usr/share/glib-2.0/valgrind/glib.supp -s --fair-sched=try\
	./programa-debug-testes Dataset_Fase1 exemplos_de_queries/tests_1/input.txt
