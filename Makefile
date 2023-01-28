OS := $(shell uname -s | tr A-Z a-z)
CC 							:= c
SOURCE 					:= source
INCLUDE 				:= include
OUT							:= build
BIN 						:= pixsim

CFDEBUG 				:= -g
CFOPT 					:= -O3
CFLAGS					:= -std=c99 $(CFDEBUG) -Wall -Wno-unused-function -Wno-switch -Wextra -pedantic -Wconversion
CLIBS 					:= -lSDL2 -lSDL2_image -lSDL2_ttf

ifeq ($(OS), darwin)
	PLATFORM			:= macosx
	CC 						:= clang
endif

ifeq ($(OS), linux)
	PLATFORM 			:= linux
	CC 						:= gcc
endif

prepare:
	mkdir build

clean_bin:
	rm $(OUT)/$(BIN)

clean:
	rm -rf $(OUT)

build_macosx:
	make prepare && $(CC) $(CFLAGS) -I./$(INCLUDE) $(SOURCE)/*.c -o $(OUT)/$(BIN) $(CLIBS)

build_linux:
	make prepare && $(CC) $(CFLAGS) -g -I./$(INCLUDE) -I./`pkg-config --cflags sdl2` $(SOURCE)/*.c -o $(OUT)/$(BIN) $(CLIBS) -Wl,-Bstatic -lSDL2 -Wl,-Bdynamic -lm -ldl -lrt

.PHONY:
	build clean
