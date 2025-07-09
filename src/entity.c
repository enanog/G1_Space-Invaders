/* ---------------------------------------------------
 * entity.c
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

#include "entity.h"

void hitboxMove(hitbox_t *hitbox, hitbox_t movement)
{
    hitbox->start.x += movement.start.x;
    hitbox->start.y += movement.start.y;
    hitbox->end.x += movement.end.x;
    hitbox->end.y += movement.end.x;
}

void hitboxPosition(hitbox_t *hitbox, hitbox_t position)
{
    hitbox->start.x = position.start.x;
    hitbox->start.y = position.start.y;
    hitbox->end.x = position.end.x;
    hitbox->end.y = position.end.y;
}