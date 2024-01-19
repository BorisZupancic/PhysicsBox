ROOT_DIR = $(shell pwd)

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
ifeq ($(PLATFORM_OS),WINDOWS)
	RAYLIB_PATH = C:/'Program Files'/raylib
	RAYGUI_PATH = C:/'Program Files'/raygui
endif


#Include paths to raylib/raygui + physics:
INCLUDE_PATHS = -I. -I$(RAYGUI_PATH)/src -I$(RAYLIB_PATH)/src  -I$(ROOT_DIR)/lib

#Links paths to raylib/raygui + physics:
LDFLAGS = -L. -L$(RAYGUI_PATH)/src  -L$(RAYLIB_PATH)/src -L$(ROOT_DIR)/lib


#Link libraries:
LDLIBS := -l:libphysics.o
ifeq ($(PLATFORM_OS),WINDOWS)
	LDLIBS += -lraylib -lwinmm -lgdi32 -lopengl32
	
	LDFLAGS += $(ROOT_DIR)/res/PhysicsBox.rc.data
endif 

ifeq ($(PLATFORM_OS), LINUX)
	LDLIBS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

# Compile ---------------------------
all: libraries
	gcc main.c -std=c11 -o $(ROOT_DIR)/bin/PhysicsBox $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)

libraries:
	cd $(ROOT_DIR)/lib && make

clean:
	cd $(ROOT_DIR)/lib && make clean
	rm -f $(ROOT_DIR)/bin/PhysicsBox.exe $(ROOT_DIR)/bin/PhysicsBox
