#include "pi_ui.h"
#include "disdrv.h"
#include <stdio.h>
#include "joydrv.h"

static input_t joy_get_input(void);
static void draw_player(void);
static void draw_enemies(void);
static void draw_barriers(void);
static void draw_hitbox_start(hitbox_t hitbox);
static void draw_hitbox_end(hitbox_t hitbox);


void pi_ui_init(void) 
{
    disp_init();
    joy_init();
}

void pi_ui_update(void) 
{
    input_t input={0,0,0,0};
    printf("Initializing game...\n");
    bool running = true;
    game_init(3,4, false);
    long long lastTime = getTimeMillis();

    while (running)
    {
        long long currentTime = getTimeMillis();
        if (currentTime - lastTime > 1000/10) // 60 FPS
        {
            //printf("ENTRO?\n");
            lastTime = currentTime;
            disp_clear();
            game_update(input);
            pi_ui_render();
            disp_update();
        }
    }
}
void pi_ui_render(void)
{
    draw_player();
    draw_enemies();
    draw_barriers();

}
   

static input_t joy_get_input(void)
{
    

}
static void draw_player(void)
{
    hitbox_t playerHitbox = getPlayerPosition();
    // Coordenada inicial del jugador
    uint8_t base_x = (uint8_t)(playerHitbox.start.x * DISP_CANT_X_DOTS);
    uint8_t base_y = (uint8_t)(playerHitbox.start.y * DISP_CANT_Y_DOTS);
    //printf("x:%f,y:%f\n",playerHitbox.start.x,playerHitbox.start.y);
    int x, y;
    for (x=0;x<PLAYER_WIDTH * DISP_CANT_X_DOTS; x++)
    {
        for (y=0;y<PLAYER_HEIGHT * DISP_CANT_Y_DOTS; y++)
        {
           // printf("basex:%d,basey:%d\n",base_x+x,base_y+y);
            if((y==0&&x==0)||(y==0 && x==(PLAYER_WIDTH*DISP_CANT_X_DOTS)-1))
                continue;
            dcoord_t cord={base_x+x,base_y+y};
            disp_write(cord,D_ON);  
        }
    }
}
static void draw_enemies(void)
{
    hitbox_t enemyHitbox;
    int row, col;
    for (row = 0; row < ENEMIES_ROW_MAX; row++)
    {
        for (col = 0; col < ENEMIES_COLUMNS_MAX; col++)
        {
           
            if (getIsEnemyAlive(row,col))
            {
                 enemyHitbox = getEnemyPosition(row, col);
            printf(" Row %d, Col %d: Start (%f, %f), End (%f, %f)\n",
                       row, col,
                       enemyHitbox.start.x, enemyHitbox.start.y,
                       enemyHitbox.end.x, enemyHitbox.end.y);
                draw_hitbox_end(enemyHitbox);
                draw_hitbox_start(enemyHitbox);
            }
        }
        printf("\n");
    }
}
static void draw_barriers(void)
{
    hitbox_t barrierHitbox;
    int barrier, row, col;
    for (barrier = 0; barrier < BARRIER_QUANTITY_MAX; barrier++)
    {
        for (row = 0; row < BARRIER_ROWS; row++)
        {
            for (col = 0; col < BARRIER_COLUMNS; col++)
            {
                if (!getBarrierIsAlive(barrier, row, col))
                {
                    continue;
                }
                barrierHitbox = getBarrierPosition(barrier, row, col);
               draw_hitbox_end(barrierHitbox);   
               draw_hitbox_start(barrierHitbox);
            }
        }
    }
}
static void draw_hitbox_end(hitbox_t hitbox)
{
    dcoord_t end = {
        .x = (uint8_t)(hitbox.end.x * DISP_CANT_X_DOTS),
        .y = (uint8_t)(hitbox.end.y * DISP_CANT_Y_DOTS)
    };
    uint8_t x;
   disp_write(end,D_ON);
}
static void draw_hitbox_start(hitbox_t hitbox)
{
    dcoord_t start = {
        .x = (uint8_t)(hitbox.start.x * DISP_CANT_X_DOTS),
        .y = (uint8_t)(hitbox.start.y * DISP_CANT_Y_DOTS)
    };
    uint8_t x;
   disp_write(start,D_ON);
}
