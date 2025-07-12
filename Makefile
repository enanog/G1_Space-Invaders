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
	SDL2_LIBS   := -lSDL2
    ALLEGRO_LIBS :=
    CFLAGS += -DRASPBERRY
else
    FRONTEND_DIR := src/frontend/pc
    FRONTEND_INC := -Iinclude/frontend/pc
    ALLEGRO_LIBS := -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_audio -lallegro_acodec
	SDL2_LIBS   :=
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
   OBJ += obj/frontend/raspberry/libAudioSDL2.o
   OBJ += obj/frontend/raspberry/joydrv.o 
   OBJ += obj/frontend/raspberry/disdrv.o
endif

# Default target
all: space_invaders

# Link final executable
space_invaders: $(OBJ)
	$(CC) -o $@ $^ $(ALLEGRO_LIBS) $(SDL2_LIBS)

# RASPBERRY specific files
ifeq ($(USER)_$(HOST),pi_raspberrypi)
obj/main.o:src/main.c include/frontend/raspberry/pi_ui.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
obj/frontend/raspberry/font.o: src/frontend/raspberry/font.c include/frontend/raspberry/font.h
	@mkdir -p obj/frontend/raspberry
	$(CC) $(CFLAGS) -c $< -o $@
obj/frontend/raspberry/pi_ui.o: src/frontend/raspberry/pi_ui.c include/frontend/raspberry/pi_ui.h include/frontend/raspberry/disdrv.h include/frontend/raspberry/joydrv.h include/game.h include/entity.h
	@mkdir -p obj/frontend/raspberry
	$(CC) $(CFLAGS) -c $< -o $@
obj/playSound.o: src/playSound.c include/playSound.h include/frontend/raspberry/audio.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
else

# PC specific files
obj/main.o:src/main.c include/frontend/pc/pc_ui.h include/playSound.h 
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
obj/frontend/pc/font.o: src/frontend/pc/font.c include/frontend/pc/font.h include/config.h include/game.h
	@mkdir -p obj/frontend/pc
	$(CC) $(CFLAGS) -c $< -o $@
obj/frontend/pc/pc_ui.o: src/frontend/pc/pc_ui.c include/frontend/pc/pc_ui.h include/game.h include/config.h include/entity.h include/playSound.h
	@mkdir -p obj/frontend/pc
	$(CC) $(CFLAGS) -c $< -o $@
obj/playSound.o: src/playSound.c include/playSound.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
endif

# General compilation rules
obj/game.o: src/game.c include/game.h include/config.h include/entity.h include/playSound.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

obj/score.o: src/score.c include/score.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Clean: remove all object and dependency files (except libAudioSDL2.o)
clean:
	@for f in obj/*.o obj/frontend/pc/*.o obj/frontend/raspberry/*.o; do \
		name=$$(basename $$f); \
		if [ "$$name" != "libAudioSDL2.o" ] && \
		   [ "$$name" != "disdrv.o" ] && \
		   [ "$$name" != "joydrv.o" ] && [ -f "$$f" ]; then \
			rm -f "$$f"; \
		fi; \
	done
	rm -f space_invaders
