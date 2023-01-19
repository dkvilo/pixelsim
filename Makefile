CC 							:= clang
SOURCE 					:= source
INCLUDE 				:= include
OUT							:= build
BIN 						:= pixsim

CFDEBUG 				:= -g
CFOPT 					:= -O3
CFLAGS					:= -std=c99 $(CFOPT) -Wall -Wno-unused-function -Wno-switch
CLIBS 					:= -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

prepare:
	mkdir build

clean_bin:
	rm $(OUT)/$(BIN)

clean:
	rm -rf $(OUT)

build:
	make prepare && $(CC) $(CFLAGS) -I./$(INCLUDE) $(SOURCE)/*.c -o $(OUT)/$(BIN) $(CLIBS)

.PHONY:
	build clean
