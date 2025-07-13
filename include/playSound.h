/* ---------------------------------------------------
 * playSound.h
 * ---------------------------------------------------
 * GRUPO 1:
 * 	CASTRO, Tomás
 *	FRIGERIO, Dylan
 * 	VALENZUELA, Agustín
 * 	YAGGI, Lucca
 *
 * Profesores:
 * 	MAGLIIOLA, Nicolas
 * 	JACOBY, Daniel
 * 	VACATELLO, Pablo
 *
 * fecha: 15/07/2025
 * ---------------------------------------------------*/
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
	SOUND_LEVELUP,
	SOUND_GAMEOVER,
	SOUND_MENU,
    SOUND_COUNT
} GameSoundEvent;

typedef enum {
	INTRO_MUSIC,
	GAME_MUSIC,
	MUSIC_COUNT
} GameMusicEvent;

// Inicializa Audio y carga los sonidos
bool playSound_init(void);

// Reproduce un sonido asociado al evento
void playSound_play(GameSoundEvent event);

void playSound_stop(GameSoundEvent event);

void playSound_restart(GameSoundEvent event);

// Libera todos los sonidos cargados
void playSound_shutdown(void);

void playSound_stopMusic(void);

void playSound_playMusic(GameMusicEvent track);

void playSound_pauseMusic(void);

void playSound_resumeMusic(void);

void playSound_setMusicVolume(float volume);

#endif

