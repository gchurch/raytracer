DEFAULT=raytracer
AA=antialiasing
BB=boundingboxes

########
#   Directories
S_DIR=src
B_DIR=build

########
#   Output
EXEC1=$(B_DIR)/$(DEFAULT)
EXEC2=$(B_DIR)/$(DEFAULT)_$(AA)
EXEC3=$(B_DIR)/$(DEFAULT)_$(BB)

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
OBJ2 = $(B_DIR)/$(DEFAULT)_$(AA).o
OBJ3 = $(B_DIR)/$(DEFAULT)_$(BB).o


########
#   Objects
$(B_DIR)/$(DEFAULT).o : $(S_DIR)/$(DEFAULT).cpp $(S_DIR)/SDLauxiliary.h $(S_DIR)/TestModel.h
	$(CC) $(CC_OPTS) -o $(B_DIR)/$(DEFAULT).o $(S_DIR)/$(DEFAULT).cpp $(SDL_CFLAGS) $(GLM_CFLAGS)

$(B_DIR)/$(DEFAULT)_$(AA).o : $(S_DIR)/$(DEFAULT)_$(AA).cpp $(S_DIR)/SDLauxiliary.h $(S_DIR)/TestModel.h
	$(CC) $(CC_OPTS) -o $(B_DIR)/$(DEFAULT)_$(AA).o $(S_DIR)/$(DEFAULT)_$(AA).cpp $(SDL_CFLAGS) $(GLM_CFLAGS)

$(B_DIR)/$(DEFAULT)_$(BB).o : $(S_DIR)/$(DEFAULT)_$(BB).cpp $(S_DIR)/SDLauxiliary.h $(S_DIR)/TestModel_boundingboxes.h
	$(CC) $(CC_OPTS) -o $(B_DIR)/$(DEFAULT)_$(BB).o $(S_DIR)/$(DEFAULT)_$(BB).cpp $(SDL_CFLAGS) $(GLM_CFLAGS)


########
#   Main build rule     
Build : $(OBJ1) Makefile
	$(CC) $(LN_OPTS) -o $(EXEC1) $(OBJ1) $(SDL_LDFLAGS)

antialiasing: $(OBJ2) Makefile
	$(CC) $(LN_OPTS) -o $(EXEC2) $(OBJ2) $(SDL_LDFLAGS)

boundingboxes: $(OBJ3) Makefile
	$(CC) $(LN_OPTS) -o $(EXEC3) $(OBJ3) $(SDL_LDFLAGS)

clean:
	rm -f $(B_DIR)/* 
