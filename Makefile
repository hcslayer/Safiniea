# Stupid simple makefile 

do : 
	gcc -std=c99 -Wall repl.c mpc.c -ledit -lm -o prompt
	