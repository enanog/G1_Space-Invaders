#include "pi_ui.h"
#include "disdrv.h"
#include <stdio.h>
#include "joydrv.h"
#include "entity.h"

static input_t joy_get_input(void);
static void draw_player(void);
static void draw_enemies(void);
static void draw_barriers(void);
static void draw_enemies_bullets(void);
static void draw_player_bullet(void);

static void draw_mothership(void);

static void draw_hitbox_filled(hitbox_t hitbox);

#define ENEMIES_ROW 2
#define ENEMIES_COLUMN 3


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
    game_init(ENEMIES_ROW,ENEMIES_COLUMN, false);
    long long lastTime = getTimeMillis();

    while (running)
    {
        input=joy_get_input();
        long long currentTime = getTimeMillis();
        if (currentTime - lastTime > 1000/60) // 60 FPS
        {
            //printf("ENTRO?\n");
            lastTime = currentTime;
            disp_clear();
           // printf(" ,%d,%d,%d\n", input.direction, input.shot, input.    pause);
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
    draw_player_bullet();
    draw_enemies_bullets();
    draw_mothership();
}

static input_t joy_get_input(void)
{
    //printf("entre\n");
    static input_t input = {0, 0, 0, 0};
    static int flag=1;
    joyinfo_t joyInfo = joy_read();

    input.direction = (joyInfo.x>40) - (joyInfo.x<-40);

    input.shot = (joyInfo.y>40 || joyInfo.y<-40);
   // printf("x:%d,y:%d,shot:%d\n",joyInfo.x,joyInfo.y,input.shot);
    //printf("direction:%d\n",input.direction);
    if (joyInfo.sw==J_PRESS)
    {
        printf("pause:%d\n",input.pause);
        printf("flag:%d\n",flag);
        if (flag)
        {
            input.pause=!input.pause;
            flag=0;
        }
    }
    else 
    {
        flag=1;
    }
    return input;
    
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
                draw_hitbox_filled(enemyHitbox);
            }
        }
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
               draw_hitbox_filled(barrierHitbox);
            }
        }
    }
}
static void draw_hitbox_filled(hitbox_t hitbox)
{
    uint8_t start_x = (uint8_t)(hitbox.start.x * DISP_CANT_X_DOTS);
    uint8_t start_y = (uint8_t)(hitbox.start.y * DISP_CANT_Y_DOTS);
    uint8_t end_x = (uint8_t)(hitbox.end.x * DISP_CANT_X_DOTS);
    uint8_t end_y = (uint8_t)(hitbox.end.y * DISP_CANT_Y_DOTS);

    // Clamp to display bounds
    if (start_x >= DISP_CANT_X_DOTS) start_x = DISP_CANT_X_DOTS - 1;
    if (end_x >= DISP_CANT_X_DOTS) end_x = DISP_CANT_X_DOTS - 1;
    if (start_y >= DISP_CANT_Y_DOTS) start_y = DISP_CANT_Y_DOTS - 1;
    if (end_y >= DISP_CANT_Y_DOTS) end_y = DISP_CANT_Y_DOTS - 1;

    for (uint8_t x = start_x; x <= end_x; x++) {
        for (uint8_t y = start_y; y <= end_y; y++) {
            dcoord_t coord = {x, y};
            disp_write(coord, D_ON);
        }
    }
}

static void draw_enemies_bullets(void)
{
    bullet_t enemyBulletBitMap[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];
    getEnemiesBulletsInfo(enemyBulletBitMap);
    int row, col;
    for (row = 0; row < ENEMIES_ROW; row++)
    {
        for (col = 0; col < ENEMIES_COLUMN; col++)
        {
            if (!enemyBulletBitMap[row][col].active)
            {
                continue;
            }
    
            hitbox_t bulletHitbox = enemyBulletBitMap[row][col].hitbox;
            draw_hitbox_filled(bulletHitbox);
        }
    }
}
static void draw_player_bullet(void)
{
    bullet_t playerBullet = getPlayerBulletinfo();
    if (playerBullet.active)
     {
        hitbox_t bulletHitbox = playerBullet.hitbox;
        draw_hitbox_filled(bulletHitbox);
    }
}
static void draw_mothership(void)
{
    if (getIsMothershipAlive())
    {
        hitbox_t mothershipHitbox = getMothershipPosition();
     
            draw_hitbox_filled(mothershipHitbox);
    }
}