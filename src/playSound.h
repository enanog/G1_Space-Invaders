#ifndef PLAY_SOUND_H
#define PLAY_SOUND_H

#include <stdbool.h>

// Enumeración de eventos de sonido
typedef enum {
    SOUND_SHOOT,
    SOUND_INVADER_KILLED,
    SOUND_EXPLOSION,
    SOUND_UFO_HIGH,
    SOUND_UFO_LOW,
    SOUND_FAST1,
    SOUND_FAST2,
    SOUND_FAST3,
    SOUND_FAST4,
    SOUND_COUNT
} GameSoundEvent;

// Inicializa Allegro Audio y carga los sonidos
bool playSound_init(void);

// Reproduce un sonido asociado al evento
void playSound_play(GameSoundEvent event);

// Libera todos los sonidos cargados
void playSound_shutdown(void);

#endif

