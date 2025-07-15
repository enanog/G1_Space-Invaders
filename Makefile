############################################################
# GROUP 1:                                                 #
#         CASTRO, Tomás                                    #
#         FRIGERIO, Dylan                                  #
#         VALENZUELA, Agusín                               #
#         YAGGI, Lucca                                     #
#                                                          #
# Professors:                                              #
#         MAGLIOLA, Nicolas                                #
#         VACATELLO, Daniel                                #
#         JACOBY, Andres                                   #
#                                                          #
# date: 08/05/2025                                         #
#                                                          #
# Makefile purpose:                                        #
# Automates compilation and linking of the Space Invaders  #
# project, handling platform-specific settings for         #
# Raspberry Pi and PC, managing source files, object       #
# files, and external libraries to produce the executable. #
############################################################

# ---------------------------------------------------
# Compiler
# ---------------------------------------------------
CC = gcc

# ---------------------------------------------------
# Platform detection: detect current user and hostname
# Used to differentiate Raspberry Pi vs PC build
# ---------------------------------------------------
USER := $(shell whoami)
HOST := $(shell hostname)

# ---------------------------------------------------
# Common compiler flags: enable all warnings, include main headers
# ---------------------------------------------------
CFLAGS = -Wall -Iinclude -Iinclude/frontend

# ---------------------------------------------------
# Platform-specific configuration:
# If running on Raspberry Pi (user=pi, host=raspberrypi), set specific directories and libs
# Else, assume PC and use Allegro libraries
# ---------------------------------------------------
ifeq ($(USER)_$(HOST),pi_raspberrypi)
	FRONTEND_DIR := src/frontend/raspberry
	FRONTEND_INC := include/frontend/raspberry
	FRONTEND_OBJ := obj/frontend/raspberry
	SDL2_LIBS   := -lSDL2
	PTHREAD_LIBS   := -lpthread
	CFLAGS += -DRASPBERRY           # Define RASPBERRY macro for conditional compilation
else
	FRONTEND_DIR := src/frontend/pc
	FRONTEND_INC := include/frontend/pc
	FRONTEND_OBJ := obj/frontend/pc
	ALLEGRO_LIBS := -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_audio -lallegro_acodec
	MATH_LIBS := -lm
endif

# ---------------------------------------------------
# Add frontend include directory to compiler flags
# ---------------------------------------------------
CFLAGS += -I$(FRONTEND_INC)

# ---------------------------------------------------
# Source files:
# - COMMON_SRC: all .c files in src/
# - FRONTEND_SRC: all .c files in platform-specific frontend directory
# Combine both for full project source list
# ---------------------------------------------------
COMMON_SRC := $(wildcard src/*.c)
FRONTEND_SRC := $(wildcard $(FRONTEND_DIR)/*.c)
SRC := $(COMMON_SRC) $(FRONTEND_SRC)

# ---------------------------------------------------
# Object files:
# Convert source file paths to object file paths inside obj/ directory
# ---------------------------------------------------
OBJ := $(patsubst src/%, obj/%, $(SRC:.c=.o))

# ---------------------------------------------------
# Additional Raspberry Pi specific object files:
# Add precompiled or manually compiled objects for audio and input drivers
# ---------------------------------------------------
ifeq ($(USER)_$(HOST),pi_raspberrypi)
   OBJ += $(FRONTEND_OBJ)/libAudioSDL2.o
   OBJ += $(FRONTEND_OBJ)/joydrv.o 
   OBJ += $(FRONTEND_OBJ)/disdrv.o
endif

# ---------------------------------------------------
# Default target to build the game executable
# ---------------------------------------------------
all: space_invaders desktop

# ---------------------------------------------------
# Raspberry Pi build rules
# ---------------------------------------------------
ifeq ($(USER)_$(HOST),pi_raspberrypi)

# Link all objects into executable with SDL2 and pthread libs
space_invaders: $(OBJ)
	$(CC) -o $@ $^ $(SDL2_LIBS) $(PTHREAD_LIBS)

# Compile main.c for Raspberry Pi, depends on pi_ui.h and playSound.h headers
obj/main.o:src/main.c $(FRONTEND_INC)/pi_ui.h include/playSound.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Compile font.c in Raspberry Pi frontend folder
$(FRONTEND_OBJ)/font.o: $(FRONTEND_DIR)/font.c $(FRONTEND_INC)/font.h
	@mkdir -p $(FRONTEND_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile pi_ui.c with dependencies
$(FRONTEND_OBJ)/pi_ui.o: $(FRONTEND_DIR)/pi_ui.c $(FRONTEND_INC)/pi_ui.h $(FRONTEND_INC)/disdrv.h $(FRONTEND_INC)/joydrv.h include/game.h include/entity.h include/playSound.h
	@mkdir -p $(FRONTEND_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile playSound.c
obj/playSound.o: src/playSound.c include/playSound.h $(FRONTEND_INC)/audio.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------------
# PC build rules
# ---------------------------------------------------
else

# Link all objects into executable with Allegro and math libraries
space_invaders: $(OBJ)
	$(CC) -o $@ $^ $(ALLEGRO_LIBS) $(MATH_LIBS)

# Compile main.c for PC, depends on pc_ui.h and playSound.h headers
obj/main.o: src/main.c $(FRONTEND_INC)/pc_ui.h include/playSound.h 
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Compile entitiesFont.c for PC frontend
$(FRONTEND_OBJ)/entitiesFont.o: $(FRONTEND_DIR)/entitiesFont.c $(FRONTEND_INC)/entitiesFont.h include/config.h include/game.h
	@mkdir -p $(FRONTEND_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile pc_ui.c with dependencies
$(FRONTEND_OBJ)/pc_ui.o: $(FRONTEND_DIR)/pc_ui.c $(FRONTEND_INC)/entitiesFont.h include/score.h include/game.h include/playSound.h
	@mkdir -p $(FRONTEND_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile playSound.c
obj/playSound.o: src/playSound.c include/playSound.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

endif

# ---------------------------------------------------
# General compilation rules for common source files
# ---------------------------------------------------

# Compile game.c
obj/game.o: src/game.c include/game.h include/config.h include/entity.h include/playSound.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Compile score.c
obj/score.o: src/score.c include/score.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------------
# Clean target:
# Remove all object files except specific precompiled ones (libAudioSDL2.o, disdrv.o, joydrv.o)
# Also removes the final executable
# ---------------------------------------------------
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

# ---------------------------------------------------
# Create .desktop launcher with icon
# ---------------------------------------------------
desktop:
	echo "[Desktop Entry]" > SpaceInvaders.desktop
	echo "Name=Space Invaders" >> SpaceInvaders.desktop
	echo "Exec=$(PWD)/space_invaders" >> SpaceInvaders.desktop
	echo "Icon=$(PWD)/assets/images/logo.png" >> SpaceInvaders.desktop
	echo "Type=Application" >> SpaceInvaders.desktop
	echo "Categories=Game;" >> SpaceInvaders.desktop
	echo "Terminal=false" >> SpaceInvaders.desktop
	chmod +x SpaceInvaders.desktop