#include "playSound.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>

static ALLEGRO_SAMPLE *sounds[SOUND_COUNT] = {0};

// Rutas a los archivos de sonido
static const char *sound_filenames[SOUND_COUNT] = {
    [SOUND_SHOOT] = "Sounds/shoot.wav",
    [SOUND_INVADER_KILLED] = "Sounds/invaderkilled.wav",
    [SOUND_EXPLOSION] = "Sounds/explosion.wav",
    [SOUND_UFO_HIGH] = "Sounds/ufo_highpitch.wav",
    [SOUND_UFO_LOW] = "Sounds/ufo_lowpitch.wav",
    [SOUND_FAST1] = "Sounds/fastinvader1.wav",
    [SOUND_FAST2] = "Sounds/fastinvader2.wav",
    [SOUND_FAST3] = "Sounds/fastinvader3.wav",
    [SOUND_FAST4] = "Sounds/fastinvader4.wav"
};

bool playSound_init(void) {
    if (!al_install_audio()) {
        fprintf(stderr, "Error: no se pudo instalar el sistema de audio.\n");
        return false;
    }

    if (!al_init_acodec_addon()) {
        fprintf(stderr, "Error: no se pudo inicializar el addon de codecs.\n");
        return false;
    }

    if (!al_reserve_samples(16)) {
        fprintf(stderr, "Error: no se pudieron reservar samples.\n");
        return false;
    }

    for (int i = 0; i < SOUND_COUNT; i++) {
        sounds[i] = al_load_sample(sound_filenames[i]);
        if (!sounds[i]) {
            fprintf(stderr, "Error cargando %s\n", sound_filenames[i]);
            return false;
        }
    }

    return true;
}

void playSound_play(GameSoundEvent event) {
    if (event >= 0 && event < SOUND_COUNT && sounds[event]) {
        al_play_sample(sounds[event], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
}

void playSound_shutdown(void) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (sounds[i]) {
            al_destroy_sample(sounds[i]);
            sounds[i] = NULL;
        }
    }
}

