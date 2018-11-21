DEFAULT=raytracer

########
#   Directories
S_DIR=src
B_DIR=build

########
#   Output
EXEC1=$(B_DIR)/$(DEFAULT)

# default build settings
CC_OPTS= -pg -Ofast -c -pipe -Wall -Wno-switch -ggdb -g3 
LN_OPTS=
CC=g++

########
#       SDL options
SDL_CFLAGS := $(shell sdl-config --cflags)
GLM_CFLAGS := -I$(GLMDIR)
SDL_LDFLAGS := $(shell sdl-config --libs)

########
#   This is the default action
all:Build


########
#   Object list
#
OBJ1 = $(B_DIR)/$(DEFAULT).o

########
#   Objects
$(B_DIR)/$(DEFAULT).o : $(S_DIR)/$(DEFAULT).cpp $(S_DIR)/SDLauxiliary.h $(S_DIR)/TestModel.h
	$(CC) $(CC_OPTS) -o $(B_DIR)/$(DEFAULT).o $(S_DIR)/$(DEFAULT).cpp $(SDL_CFLAGS) $(GLM_CFLAGS)


########
#   Main build rule     
Build : $(OBJ1) Makefile
	$(CC) $(LN_OPTS) -o $(EXEC1) $(OBJ1) $(SDL_LDFLAGS)

run: ./$(B_DIR)/raytracer Makefile
	./$(B_DIR)/raytracer


clean:
	rm -f $(B_DIR)/* 
