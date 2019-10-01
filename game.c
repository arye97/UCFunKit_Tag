#include "pacer.h"
#include "tinygl.h"
#include "system.h"
#include "navswitch.h"
#include "ir_uart.h"
#include "../fonts/font5x7_1.h"
#define MESSAGE_RATE 20

typedef struct player_point
{
    //player coordinates
    tinygl_coord_t x;
    tinygl_coord_t y;
    int tagger; // 1 if tagger, 0 if not
} player_point_t; 



int getPlayer_x(player_point_t player1)
{
    //constants
    //max and min x values for sides of the board
    int max_x = 4;
    int min_x = 0;
    int turn_off = 0;
    if (navswitch_push_event_p (NAVSWITCH_EAST)) {
        //set the pixel you're leaving turned off
        tinygl_pixel_set(tinygl_point (player1.x, player1.y), turn_off);
        if (player1.x != max_x) {
            player1.x = (player1.x + 1);
        }
    }

    if (navswitch_push_event_p (NAVSWITCH_WEST)) {
        //set the pixel you're leaving turned off
        tinygl_pixel_set(tinygl_point (player1.x, player1.y), turn_off);
        if (player1.x != min_x) {
            player1.x = (player1.x - 1);
        }
    }

    return player1.x;
}


int getPlayer_y(player_point_t player1)
{
    /////effects y
    int max_y = 6;
    int min_y = 0;
    int turn_off = 0;
    if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
        //set the pixel you're leaving turned off
        tinygl_pixel_set(tinygl_point (player1.x, player1.y), turn_off);
        if (player1.y != max_y) {
            player1.y = (player1.y + 1);
        }
    }
    if (navswitch_push_event_p (NAVSWITCH_NORTH)) {
        //set the pixel you're leaving turned off
        tinygl_pixel_set(tinygl_point (player1.x, player1.y), turn_off);
        if (player1.y != min_y) {
            player1.y = (player1.y - 1);
        }
    }
    return player1.y;
}

int coord_to_int(int x, int y)
{
    /* encodes the coordinates
     * into one number to be 
     * sent by the ir blaster
     */
    int encoder = 5;
    int num = x + encoder * y;
    return num;
}

void zero_player(player_point_t player)
{
    //sets pixel player was previously on, to off
    tinygl_pixel_set(tinygl_point (player.x, player.y), 0);
}

player_point_t receive_1(int coord_num, player_point_t player1)
{
    /* recieves player1 coordinates from player1
     * sets it on player2's board
     */ 
    zero_player(player1);
    int y = 0;
    while (coord_num > 4) {
        coord_num -= 5;
        y += 1;
    }
    player1.x = coord_num;
    player1.y = y;
    return player1;
}

player_point_t receive_2(int coord_num, player_point_t player2)
{
    /* recieves player2 coordinates from player2
     * sets it on player1's board
     */
    zero_player(player2);
    int y = 0;
    while (coord_num > 4) {
        coord_num -= 5;
        y += 1;
    }
    player2.x = coord_num;
    player2.y = y;
    return player2;
}

void display_players(player_point_t player1, player_point_t player2)
{
    //displays both players on the board
    tinygl_pixel_set(tinygl_point (player1.x, player1.y), 1);
    tinygl_pixel_set(tinygl_point (player2.x, player2.y), 1);
    tinygl_update();
}


player_point_t set_player1(player_point_t player1)
{
    //sets player1 coordinates to its origin
    player1.x = 4;
    player1.y = 6;
    return player1;
}

player_point_t set_player2(player_point_t player2)
{
    //sets player2 coordinates to its origin
    player2.x = 0;
    player2.y = 0;
    return player2;
}

int main (void)
{
    system_init();
    ir_uart_init();
    pacer_init (1000);
    tinygl_init (1000);
    navswitch_init();

    //initialise player1
    player_point_t player1;
    player1.x = 4;
    player1.y = 6;
    player1.tagger = 1;

    //initialise player2
    player_point_t player2;
    player2.x = 0;
    player2.y = 0;
    player2.tagger = 0;

    //unique variables
    int tagged = 0;
    int gameStart = 0;
    int isPlayer1 = 0;
    int max_tags = 30;
    //set tinygl text feature
    tinygl_font_set (&font5x7_1);
    tinygl_text_speed_set (10);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("PLAY TAG");

    //runs the scrolling "PLAY TAG" text
    while (gameStart != 1) {
        navswitch_update();
        pacer_wait();
        tinygl_update();
        if (gameStart == 0) { //game hasnt started
            if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
                isPlayer1 = 1;
                gameStart = 1; 
                ir_uart_putc(gameStart);
            }
            if (ir_uart_read_ready_p()) {
                gameStart = 1;
            }
        }
    }
    tinygl_clear(); //get rid of text
    if (gameStart == 1) {
        while (1)
        {
            navswitch_update();
            pacer_wait ();

            if (gameStart == 1) {

                // this code is executed while the game is in play

                if (isPlayer1 == 1) { // player1's board reads this code
                    if (ir_uart_read_ready_p ()) {
                        zero_player(player2);
                        player2 = receive_1(ir_uart_getc (), player1);
                    }

                    if ((player1.x != getPlayer_x(player1)) || (player1.y != getPlayer_y(player1))) {
                        player1.x = getPlayer_x(player1);
                        player1.y = getPlayer_y(player1);

                        ir_uart_putc(coord_to_int(player1.x, player1.y));
                    }
                }

                if (isPlayer1 == 0) { // player2's board reads this code
                    if (ir_uart_read_ready_p ()) {
                        zero_player(player1);
                        //recieves player1 coords
                        player1 = receive_2(ir_uart_getc (), player2);
                        
                    }

                    if ((player2.x != getPlayer_x(player2)) || (player2.y != getPlayer_y(player2))) {
                        player2.x = getPlayer_x(player2);
                        player2.y = getPlayer_y(player2);
                        //sends player2 coords
                        ir_uart_putc(coord_to_int(player2.x, player2.y));
                    }
                }


            // if collision
            if ((player1.x ==  player2.x) && (player1.y == player2.y)) {
                tagged += 1;
                zero_player(player1);
                zero_player(player2);
                player1 = set_player1(player1);
                player2 = set_player2(player2);
            }
            //if is the end
            if (tagged >= max_tags) {
                //resets game
                gameStart = 0;
                tinygl_pixel_set(tinygl_point (player1.x, player1.y), 0);
                tinygl_pixel_set(tinygl_point (player2.x, player2.y), 0);

                if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
                    gameStart = 1;
                    tagged = 0;
                    ir_uart_putc(gameStart);
                    player1 = set_player1(player1);
                    player2 = set_player2(player2);
                }
                if (ir_uart_read_ready_p ()) {
                    gameStart = ir_uart_getc();
                    tagged = 0;
                    player1 = set_player1(player1);
                    player2 = set_player2(player2);
                }

            }

            display_players(player1, player2);
            tinygl_update();
            }
        }
    }
}
