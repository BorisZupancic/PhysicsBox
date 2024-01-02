all:
	gcc -std=c11 -o main main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
