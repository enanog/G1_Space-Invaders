GC := gcc
CFLAGS := -Wall
ALLEGRO_LIBS := -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives

OBJ := game.o pc_ui.o

all: out

out: game.o pc_ui.o
	${GC} ${CFLAGS} ${OBJ} -o out ${ALLEGRO_LIBS}

pc_ui.o: pc_ui.c game.h config.h
	${GC} ${CFLAGS} -c pc_ui.c

game.o: game.c game.h entity.h config.h
	${GC} ${CFLAGS} -c game.c

clean:
	rm -f *.o out
