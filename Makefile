CC = gcc

# Compilation flags:
# -Wall: enable all warnings
# -Iinclude: add 'include/' directory to the list of paths for header files
CFLAGS = -Wall -Iinclude -Iinclude/frontend

# Detect platform using whoami and hostname
USER := $(shell whoami)
HOST := $(shell hostname)
# Check if user and hostname match the expected values for Raspberry Pi
ifeq ($(USER)_$(HOST),pi_raspberrypi)
    FRONTEND_DIR := src/frontend/raspberry
    FRONTEND_INC := -Iinclude/frontend/raspberry
    # Allegro libraries
	ALLEGRO_LIBS := 
	CFLAGS += -DRASPBERRY
	# Object files
	OBJ = $(patsubst src/%, obj/%, $(SRC:.c=.o))
    OBJ += obj/frontend/raspberry/libAudioSDL2.o
else
    FRONTEND_DIR := src/frontend/pc
    FRONTEND_INC := -Iinclude/frontend/pc
    # Allegro libraries
    ALLEGRO_LIBS = -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_audio -lallegro_acodec
    # Object files
	OBJ = $(patsubst src/%, obj/%, $(SRC:.c=.o))
endif

# Add platform-specific includes
CFLAGS += $(FRONTEND_INC)

# Source files
COMMON_SRC := $(wildcard src/*.c)
FRONTEND_SRC := $(wildcard $(FRONTEND_DIR)/*.c)
SRC := $(COMMON_SRC) $(FRONTEND_SRC)

# Default target
all: space_invaders

# Link executable
space_invaders: $(OBJ)
	$(CC) -o $@ $^ $(ALLEGRO_LIBS)

# Compile src/*.c
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile frontend files
obj/frontend/raspberry/%.o: src/frontend/raspberry/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/frontend/pc/%.o: src/frontend/pc/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	find obj/ -type f ! -path 'obj/frontend/raspberry/libAudioSDL2.o' -name '*.o' -delete
	rm -f $(OBJ) space_invaders
