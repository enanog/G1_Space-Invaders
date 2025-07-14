#ifndef PI_UI_H
#define PI_UI_H

#include "config.h"
#include "game.h"
#include <stdbool.h>

/**
 * @brief Initialize the PI UI system
 */
void pi_ui_init(void);

/**
 * @brief Main menu loop
 */
void pi_ui_menu(void);
#endif // PI_UI_H