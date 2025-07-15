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
 * 	MAGLIOLA, Nicolas
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
	SOUND_DEATH,
	SOUND_MOTHERSHIPDEATH,
    SOUND_COUNT
} GameSoundEvent;

typedef enum {
	INTRO_MUSIC,
	GAME_MUSIC,
	MUSIC_COUNT
} GameMusicEvent;

/* ---------------------------------------------------
 * @brief Initialize the audio system and load sound effects
 * 
 * Installs Allegro audio, initializes codecs, reserves sample slots,
 * and loads each sound effect from the corresponding file.
 * 
 * @return true if all initialization and loading succeeded, false otherwise
 * ---------------------------------------------------*/
bool playSound_init(void);

/* ---------------------------------------------------
 * @brief Play a sound effect given a sound event enum
 * 
 * Plays the corresponding sound sample once at full volume and center pan.
 * If the sound event corresponds to the UFO low-pitch sound, sets a flag.
 * 
 * @param event Enum value of the sound to play
 * ---------------------------------------------------*/
void playSound_play(GameSoundEvent event);

/* ---------------------------------------------------
 * @brief Stop a currently playing sound effect by event enum
 * 
 * Stops the sample instance identified by the event.
 * If stopping the UFO low-pitch sound, clears the flag.
 * 
 * @param event Enum value of the sound to stop
 * 
 * RASPBERRY:
 *          No implementation: SDL Simple Audio does not allow stopping individual samples.
 * ---------------------------------------------------*/
void playSound_stop(GameSoundEvent event);

/* ---------------------------------------------------
 * @brief Clean up and free all loaded sound effects and music
 * 
 * Stops music playback and destroys all loaded samples,
 * resetting internal state flags.
 * ---------------------------------------------------*/
void playSound_shutdown(void);

/* ---------------------------------------------------
 * @brief Restart a sound effect by stopping and playing again
 * 
 * Useful for re-triggering a sound that might still be playing.
 * 
 * @param event Enum value of the sound to restart
 * RASPBERRY:
 *          No implementation: SDL Simple Audio does not allow stopping individual samples.
 * ---------------------------------------------------*/
void playSound_restart(GameSoundEvent event);

/* ---------------------------------------------------
 * @brief Start playing a music track by enum value
 * 
 * Stops any currently playing music, loads and starts
 * the new track in loop mode at full volume.
 * Does nothing if the requested track is already playing.
 * 
 * @param track Enum value of the music track to play
 * ---------------------------------------------------*/
void playSound_playMusic(GameMusicEvent track);

/* ---------------------------------------------------
 * @brief Stop any currently playing music and free resources
 * RASPBERRY:
 *          No implementation: SDL Simple Audio does not allow stopping individual samples.
 * ---------------------------------------------------*/
void playSound_stopMusic(void);

/* ---------------------------------------------------
 * @brief Pause the currently playing music stream
 * 
 * Also stops the UFO low pitch sound if it was playing.
 * ---------------------------------------------------*/
void playSound_pauseMusic(void);

/* ---------------------------------------------------
 * @brief Resume playback of the paused music stream
 * 
 * Also resumes the UFO low pitch sound if it was playing.
 * ---------------------------------------------------*/
void playSound_resumeMusic(void);

/* ---------------------------------------------------
 * @brief Set the playback volume of the current music stream
 * 
 * Volume range: 0.0 (mute) to 1.0 (full volume)
 * 
 * @param volume Desired music volume level
 * 
 * RASPBERRY:
 *          No implementation: SDL Simple Audio does not allow stopping individual samples.
 * ---------------------------------------------------*/
void playSound_setMusicVolume(float volume);

#endif

