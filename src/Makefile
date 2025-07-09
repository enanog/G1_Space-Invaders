GC := gcc
CFLAGS := -Wall
ALLEGRO_LIBS := -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_audio -lallegro_acodec

OBJ := game.o pc_ui.o playSound.o

all: out

out: ${OBJ}
	${GC} ${CFLAGS} ${OBJ} -o out ${ALLEGRO_LIBS}

pc_ui.o: pc_ui.c game.h config.h 
	${GC} ${CFLAGS} -c pc_ui.c

game.o: game.c game.h entity.h config.h
	${GC} ${CFLAGS} -c game.c

playSound.o: playSound.c playSound.h
	${GC} ${CFLAGS} -c playSound.c

clean:
	rm -f *.o out
