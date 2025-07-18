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

#include "playSound.h"
#include <stdio.h>
#include <stdbool.h>


// Paths to sound effect files, indexed by sound event enum values
static const char *sound_filenames[SOUND_COUNT] = 
{
    [SOUND_SHOOT] = "assets/sounds/shoot.wav",
    [SOUND_INVADER_KILLED] = "assets/sounds/invaderkilled.wav",
    [SOUND_EXPLOSION] = "assets/sounds/explosion.wav",
    [SOUND_UFO_HIGH] = "assets/sounds/ufo_highpitch.wav",
    [SOUND_UFO_LOW] = "assets/sounds/ufo_lowpitch.wav",
    [SOUND_FAST1] = "assets/sounds/fastinvader1.wav",
    [SOUND_FAST2] = "assets/sounds/fastinvader2.wav",
    [SOUND_FAST3] = "assets/sounds/fastinvader3.wav",
    [SOUND_FAST4] = "assets/sounds/fastinvader4.wav",
	[SOUND_LEVELUP] = "assets/sounds/levelup.wav",
	[SOUND_GAMEOVER] = "assets/sounds/gameover.wav",
	[SOUND_MENU] = "assets/sounds/menu.wav",
	[SOUND_DEATH] = "assets/sounds/death.wav",
	[SOUND_MOTHERSHIPDEATH] = "assets/sounds/mothershipdeath.wav",
};

// Paths to music track files, indexed by music track enum values
static const char *music_filenames[MUSIC_COUNT] =
{
    [INTRO_MUSIC] = "assets/sounds/spaceinvaders2.wav",
    [GAME_MUSIC] = "assets/sounds/spaceinvaders1.wav"
};

#ifndef RASPBERRY

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

// Array holding loaded sound samples for all sound effects
static ALLEGRO_SAMPLE *sounds[SOUND_COUNT] = {0};

// IDs used to control individual samples playing
static ALLEGRO_SAMPLE_ID sound_ids[SOUND_COUNT];

// Current music audio stream playing
static ALLEGRO_AUDIO_STREAM *music_stream = NULL;

// Tracks which music is currently playing (-1 means none)
static GameMusicEvent current_music = -1;

// Flag to track if the UFO low pitch sound is playing (special sound)
static bool motherWasPlaying = false;

bool playSound_init(void) 
{
    if (!al_install_audio()) 
    {
        fprintf(stderr, "Error: no se pudo instalar el sistema de audio.\n");
        return false;
    }

    if (!al_init_acodec_addon()) 
    {
        fprintf(stderr, "Error: no se pudo inicializar el addon de codecs.\n");
        return false;
    }

    if (!al_reserve_samples(16)) 
    {
        fprintf(stderr, "Error: no se pudieron reservar samples.\n");
        return false;
    }

    for (int i = 0; i < SOUND_COUNT; i++) 
    {
        sounds[i] = al_load_sample(sound_filenames[i]);
        if (!sounds[i]) 
        {
            fprintf(stderr, "Error cargando %s\n", sound_filenames[i]);
            return false;
        }
    }

    return true;
}

void playSound_play(GameSoundEvent event) 
{
    if (event >= 0 && event < SOUND_COUNT && sounds[event]) 
    {
        al_play_sample(sounds[event], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_ids[event]);
    }

    if (event == SOUND_UFO_LOW)
    {
    	motherWasPlaying = true;
    }
}

void playSound_stop(GameSoundEvent event)
{
    if (event >= 0 && event < SOUND_COUNT)
    {
        al_stop_sample(&sound_ids[event]);
    }

    if (event == SOUND_UFO_LOW)
	{
		motherWasPlaying = false;
	}
}

void playSound_shutdown(void) 
{
	playSound_stopMusic();
    for (int i = 0; i < SOUND_COUNT; i++) 
    {
    	motherWasPlaying = false;
        if (sounds[i]) 
        {
            al_destroy_sample(sounds[i]);

            sounds[i] = NULL;
        }
    }
}

void playSound_restart(GameSoundEvent event)
{
    if (event >= 0 && event < SOUND_COUNT && sounds[event])
    {
        // Stop the sound if it was already playing
        al_stop_sample(&sound_ids[event]);

        // Play it again from the beginning
        al_play_sample(sounds[event], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &sound_ids[event]);
    }
}


void playSound_playMusic(GameMusicEvent track)
{
    if (track < 0 || track >= MUSIC_COUNT)
        return;

    // Si ya está sonando la misma música, no la reiniciamos
    if (music_stream && current_music == track)
        return;

    // Si hay música sonando, la paramos
    if (music_stream)
        playSound_stopMusic();

    music_stream = al_load_audio_stream(music_filenames[track], 4, 2048);
    if (!music_stream)
    {
        fprintf(stderr, "Error: No se pudo cargar música %s\n", music_filenames[track]);
        return;
    }

    current_music = track;
    al_attach_audio_stream_to_mixer(music_stream, al_get_default_mixer());
    al_set_audio_stream_playmode(music_stream, ALLEGRO_PLAYMODE_LOOP);
    al_set_audio_stream_gain(music_stream, 1.0);
}

void playSound_stopMusic(void)
{
    if (music_stream)
    {
        al_destroy_audio_stream(music_stream);
        music_stream = NULL;
        current_music = -1;
    }
}

void playSound_pauseMusic(void)
{
    if (music_stream)
        al_set_audio_stream_playing(music_stream, false);

    if(motherWasPlaying)
	{
    	al_stop_sample(&sound_ids[SOUND_UFO_LOW]);
	}
}

void playSound_resumeMusic(void)
{
    if (music_stream)
        al_set_audio_stream_playing(music_stream, true);

    if(motherWasPlaying)
    {
    	playSound_play(SOUND_UFO_LOW);
    }
}

void playSound_setMusicVolume(float volume)
{
    if (music_stream)
        al_set_audio_stream_gain(music_stream, volume); // entre 0.0 y 1.0
}

#else

#include <pthread.h>
#include "audio.h"

static char musicPlaying = MUSIC_COUNT;
static bool isPaussed = 0;

bool playSound_init(void)
{
	fflush(stdout);
	//Init Simple-SDL2-Audio */
	if ( initAudio() == NO_INIT)
	{
		printf("Audio not initilized.\n");
		endAudio();
		return false;
	}
    return true;
}

void playSound_play(GameSoundEvent event) 
{
	playSound(sound_filenames[event], SDL_MIX_MAXVOLUME);
}

void playSound_stop(GameSoundEvent event)
{
    // // Simple-SDL2-Audio no permite detener un único sonido específico,
    // // así que usamos pauseAudio() para pausar el dispositivo entero.
    //pauseAudio();
}

void playSound_shutdown(void) 
{
    endAudio();
}

void playSound_restart(GameSoundEvent event)
{
    //
}

void playSound_playMusic(GameMusicEvent track)
{
	if(musicPlaying != track)
	{
		playMusic(music_filenames[track], SDL_MIX_MAXVOLUME);
		musicPlaying = track;
	}

}

void playSound_stopMusic(void)
{

}

void playSound_pauseMusic(void)
{
	if(isPaussed == 0)
	{
		pauseAudio();
		isPaussed = 1;
	}
}

void playSound_resumeMusic(void)
{
	if(isPaussed == 1)
	{
		unpauseAudio();
		isPaussed = 0;
	}

}

void playSound_setMusicVolume(float volume)
{

}

#endif

