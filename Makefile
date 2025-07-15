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
	FRONTEND_INC := include/frontend/raspberry
	FRONTEND_OBJ := obj/frontend/raspberry
	SDL2_LIBS   := -lSDL2
	PTHREAD_LIBS   := -lpthread
	CFLAGS += -DRASPBERRY
else
	FRONTEND_DIR := src/frontend/pc
	FRONTEND_INC := include/frontend/pc
	FRONTEND_OBJ := obj/frontend/pc
	ALLEGRO_LIBS := -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_audio -lallegro_acodec
	MATH_LIBS := -lm
endif

# Add frontend includes to flags
CFLAGS += -I$(FRONTEND_INC)

# Source files (exclude libAudioSDL2.c if exists in raspberry)
COMMON_SRC := $(wildcard src/*.c)
FRONTEND_SRC := $(wildcard $(FRONTEND_DIR)/*.c)
SRC := $(COMMON_SRC) $(FRONTEND_SRC)

# Object files
OBJ := $(patsubst src/%, obj/%, $(SRC:.c=.o))

# Manually add precompiled .o if on Raspberry
ifeq ($(USER)_$(HOST),pi_raspberrypi)
   OBJ += $(FRONTEND_OBJ)/libAudioSDL2.o
   OBJ += $(FRONTEND_OBJ)/joydrv.o 
   OBJ += $(FRONTEND_OBJ)/disdrv.o
endif

# Default target
all: space_invaders

# RASPBERRY specific files
ifeq ($(USER)_$(HOST),pi_raspberrypi)
space_invaders: $(OBJ)
	$(CC) -o $@ $^ $(SDL2_LIBS) $(PTHREAD_LIBS)

obj/main.o:src/main.c $(FRONTEND_INC)/pi_ui.h include/playSound.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
$(FRONTEND_OBJ)/font.o: $(FRONTEND_DIR)/font.c $(FRONTEND_INC)/font.h
	@mkdir -p $(FRONTEND_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
$(FRONTEND_OBJ)/pi_ui.o: s$(FRONTEND_DIR)/pi_ui.c $(FRONTEND_INC)/pi_ui.h $(FRONTEND_INC)/disdrv.h $(FRONTEND_INC)/joydrv.h include/game.h include/entity.h include/playSound.h
	@mkdir -p $(FRONTEND_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
obj/playSound.o: src/playSound.c include/playSound.h $(FRONTEND_INC)/audio.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
else
space_invaders: $(OBJ)
	$(CC) -o $@ $^ $(ALLEGRO_LIBS) $(MATH_LIBS)
# PC specific files
obj/main.o: src/main.c $(FRONTEND_INC)/pc_ui.h include/playSound.h 
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
$(FRONTEND_OBJ)/entitiesFont.o: $(FRONTEND_DIR)/entitiesFont.c $(FRONTEND_INC)/entitiesFont.h include/config.h include/game.h
	@mkdir -p $(FRONTEND_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
$(FRONTEND_OBJ)/pc_ui.o: $(FRONTEND_DIR)/pc_ui.c $(FRONTEND_INC)/entitiesFont.h include/game.h include/config.h include/entity.h include/playSound.h
	@mkdir -p $(FRONTEND_OBJ)
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
