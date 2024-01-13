ifeq ($(OS),Windows_NT)
	PLATFORM_OS = WINDOWS
else
	UNAMEOS = $(shell uname)
	ifeq ($(UNAMEOS),Linux)
		PLATFORM_OS = LINUX
	endif
endif

RAYLIB_PATH = C:/'Program Files'/raylib
RAYGUI_PATH = C:/'Program Files'/raygui

INCLUDE_PATHS = -I. -I$(RAYGUI_PATH)/src -I$(RAYLIB_PATH)/src 
LDFLAGS = -L. -L$(RAYGUI_PATH)/src  -L$(RAYLIB_PATH)/src

LDLIBS :=
ifeq ($(PLATFORM_OS),WINDOWS)
	LDLIBS += -lraylib -lwinmm -lgdi32 -lopengl32
endif 

ifeq ($(PLATFORM_OS), LINUX)
	LDLIBS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

#linux:
#	gcc -g -std=c11 -o main main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
#windows: main.c
#	gcc main.c -std=c11 -o PhysicsBox_Windows.exe -I C:/'Program Files'/raygui/src -L C:/'Program Files'/raygui/src -I C:/'Program Files'/raylib/src -L C:/'Program Files'/raylib/src -lraylib -lwinmm -lgdi32 -lopengl32
all:
	gcc main.c -std=c11 -o PhysicsBox_$(PLATFORM_OS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)