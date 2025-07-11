# Compiler
CC = gcc

# Platform detection
USER := $(shell whoami)
HOST := $(shell hostname)

# Flags
CFLAGS = -Wall -Iinclude -Iinclude/frontend

# Platform-specific config
ifeq ($(USER)_$(HOST),pi_raspberrypi)
    FRONTEND_DIR := src/frontend/raspberry
    FRONTEND_INC := -Iinclude/frontend/raspberry
    ALLEGRO_LIBS :=
    CFLAGS += -DRASPBERRY
else
    FRONTEND_DIR := src/frontend/pc
    FRONTEND_INC := -Iinclude/frontend/pc
    ALLEGRO_LIBS = -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_audio -lallegro_acodec
endif

# Add frontend includes to flags
CFLAGS += $(FRONTEND_INC)

# Source files (exclude libAudioSDL2.c if exists in raspberry)
COMMON_SRC := $(wildcard src/*.c)
FRONTEND_SRC := $(wildcard $(FRONTEND_DIR)/*.c)
SRC := $(COMMON_SRC) $(FRONTEND_SRC)

# Object files
OBJ := $(patsubst src/%, obj/%, $(SRC:.c=.o))

# Manually add precompiled .o if on Raspberry
ifeq ($(USER)_$(HOST),pi_raspberrypi)
   # OBJ += obj/frontend/raspberry/libAudioSDL2.o
   OBJ += obj/frontend/raspberry/joydrv.o 
   OBJ += obj/frontend/raspberry/disdrv.o
endif

# Default target
all: space_invaders

# Link final executable
space_invaders: $(OBJ)
	$(CC) -o $@ $^ $(ALLEGRO_LIBS)

# Compilation rules
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/frontend/raspberry/%.o: src/frontend/raspberry/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/frontend/pc/%.o: src/frontend/pc/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean: remove all object and dependency files (except libAudioSDL2.o)
clean:
	rm -f $(filter-out obj/frontend/raspberry/libAudioSDL2.o, $(wildcard obj/**/*.o))
	rm -f space_invaders
