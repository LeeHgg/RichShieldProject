#include <zephyr/drivers/led.h>
#include <zephyr/drivers/kscan.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

#define LED_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht16k33)
#define KEY_NODE DT_CHILD(LED_NODE, keyscan)

#define MAX_LED_NUM 128

#define MAX_ROTARY_IDX 15 //rotary

static const struct device *const led = DEVICE_DT_GET(LED_NODE);

int led_init(int current_led_index);
void led_off_all(void);
void turn_on_walls(int *walls, int walls_count);
void led_off_without_walls(int *walls, int walls_count);

int led_on_right(int current_led_index);
int led_on_left(int current_led_index);
int led_on_up(int current_led_index);
int led_on_down(int current_led_index);

void led_display_game_over(void);
void led_display_game_clear(void);

