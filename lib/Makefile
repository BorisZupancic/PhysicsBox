CC = gcc
CFLAGS = -Wall -g 

#Determine OS
ifeq ($(OS),Windows_NT)
	PLATFORM_OS = WINDOWS
else
	UNAMEOS = $(shell uname)
	ifeq ($(UNAMEOS),Linux)
		PLATFORM_OS = LINUX
	endif
endif

#Paths to Dependencies (raylib/raygui)
RAYLIB_PATH = C:/'Program Files'/raylib
RAYGUI_PATH = C:/'Program Files'/raygui

#Include paths to raylib/raygui:
INCLUDE_PATHS = -I. -I$(RAYGUI_PATH)/src -I$(RAYLIB_PATH)/src 

#Links paths to raylib/raygui:
LDFLAGS = -L. -L$(RAYGUI_PATH)/src  -L$(RAYLIB_PATH)/src


#Link libraries:
LDLIBS :=
ifeq ($(PLATFORM_OS),WINDOWS)
	LDLIBS += -lraylib -lwinmm -lgdi32 -lopengl32
endif 
ifeq ($(PLATFORM_OS), LINUX)
	LDLIBS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

all: libtest

libphysics.o: libphysics.c physics.h
	$(CC) $(CFLAGS) -c libphysics.c $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)

libtest: libtest.c libphysics.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)

clean:
	rm *.o libtest.exe libtest