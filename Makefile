CC = gcc

# Compilation flags:
# -Wall: enable all warnings
# -Iinclude: add 'include/' directory to the list of paths for header files
CFLAGS = -Wall -Iinclude -Iinclude/frontend

# Find all .c source files in src/ and src/frontend/
SRC = $(wildcard src/*.c src/frontend/*.c)

# Generate corresponding .o object file paths inside obj/
OBJ = $(patsubst src/%, obj/%, $(SRC:.c=.o))

# Allegro libraries required for linking
ALLEGRO_LIBS = -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_audio -lallegro_acodec 

# Default target: build the final executable
all: space_invaders_pc

# Link all object files to create the executable 'space_invaders_pc'
space_invaders_pc: $(OBJ)
	$(CC) -o $@ $^ $(ALLEGRO_LIBS)

# Generic rule to compile .c files from src/ into obj/
obj/%.o: src/%.c
	@mkdir -p $(dir $@)             # Create the directory if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@     # Compile source to object

# Generic rule to compile .c files from src/frontend/ into obj/
obj/%.o: src/frontend/%.c
	@mkdir -p $(dir $@)             # Create the directory if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@     # Compile source to object

# Remove all object files and the executable
clean:
	rm -f $(OBJ) space_invaders_pc
