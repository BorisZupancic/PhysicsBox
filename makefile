linux:
	gcc -g -std=c11 -o main main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
windows: 
	gcc main.c -std=c11 -o main.exe -I C:/'Program Files'/raygui/src -L C:/'Program Files'/raygui/src -I C:/'Program Files'/raylib/src -L C:/'Program Files'/raylib/src -lraylib -lwinmm -lgdi32 -lopengl32
