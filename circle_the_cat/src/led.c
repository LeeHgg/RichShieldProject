#include "led.h"

// int number_led_matrix_arr [MAX_LED_NUM];

int number_led_matrix_arr [MAX_LED_NUM] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,
    0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,
    0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void led_display_game_over() {
    led_off_all();
    for (int i = 0; i < MAX_LED_NUM; i++) {
        if (number_led_matrix_arr[i] == 1) {
            led_on(led, i);
        } else {
            led_off(led, i);
        }
    }
}

int number_led_matrix_arr_clear [MAX_LED_NUM] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,
    0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,
    0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0
};

void led_display_game_clear() {
    for (int i = 0; i < 3; i++) {
        led_off_all();
        k_sleep(K_MSEC(500));
        for (int i = 0; i < MAX_LED_NUM; i++) {
            if (number_led_matrix_arr_clear[i] == 1) {
                led_on(led, i);
            } else {
                led_off(led, i);
            }
        }
        k_sleep(K_MSEC(500));
    }
    
}


int led_init(int start_index)
{
    if(!device_is_ready(led)){
        printk("LED device %s is not ready\n", led->name);
    }
    led_off_all();
    led_on(led, start_index);

    return 0;
}

void led_off_without_walls(int *walls, int walls_count)
{
    for(int i = 0; i< MAX_LED_NUM; i++){
        led_off(led, i);
    }
    turn_on_walls(walls, walls_count);
}

void led_off_all()
{
    for(int i = 0; i< MAX_LED_NUM; i++){
        led_off(led, i);
    }
}

void turn_on_walls(int *walls, int walls_count)
{
    for (int i = 0; i < walls_count; i++)
    {
        led_on(led, walls[i]);
    }
}


int led_on_right(int current_led_index)
{
    if ((current_led_index-15) % 16 == 0) current_led_index -= 16;
    current_led_index += 1;
    
    led_on(led, current_led_index);
    

    k_sleep(K_MSEC(100));
    return current_led_index;
}


int led_on_left(int current_led_index)
{

    if (current_led_index % 16 == 0) current_led_index += 15;   
    current_led_index -= 1;
    
    led_on(led, current_led_index);

    k_sleep(K_MSEC(100));
    return current_led_index;
}


int led_on_up(int current_led_index)
{

    if (current_led_index <= 15) current_led_index += 128;
    current_led_index -= 16;

    led_on(led, current_led_index);

    k_sleep(K_MSEC(100));
    return current_led_index;
}


int led_on_down(int current_led_index)
{

    if (current_led_index >= 112) current_led_index -= 128;
   current_led_index += 16;

    led_on(led, current_led_index);

    k_sleep(K_MSEC(100));
    return current_led_index;

}



// * * * * * * * * * * * * * * * *  0 ~ 15
// * * * * * * * * * * * * * * * * 16 ~ 31
// * * * * * * * * * * * * * * * * 32 ~ 47
// x x x x x x x x * * * * * * * * 48 ~ 63 (48 ~ 55)
// * * * * * * * * x x x x x x x x 64 ~ 79 (72 ~ 79)
// * * * * * * * * * * * * * * * * 80 ~ 95
// * * * * * * * * * * * * * * * * 96 ~ 111
// * * * * * * * * * * * * * * * * 112 ~ 127 