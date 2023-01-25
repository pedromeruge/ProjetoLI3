#!/usr/bin/env python3

# Using readlines()
file1 = open('testes_output_final.txt', 'r')
Lines = file1.readlines()
  
CPUtime_load = 0
Walltime_load = 0
CPUtime_free = 0
Walltime_free = 0
# Strips the newline character
i = 0
for line in Lines:
	a, b = line.split(" ")
	if (i % 2) == 0:
		CPUtime_load += float(a)
		Walltime_load += float(b)
	else:
		CPUtime_free += float(a)
		Walltime_free += float(b)
	i += 1

total = len(Lines) / 2
print("Time to load data:\nCPU:{0} Wall clock:{1}".format(CPUtime_load / total, Walltime_load / total))
print("Time to free data:\nCPU:{0} Wall clock:{1}".format(CPUtime_free / total, Walltime_free / total))
file1.close()
