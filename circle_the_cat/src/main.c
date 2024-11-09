#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>

#include "led.h"
#include "batterydisplay.h"
#include "value.h"

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
   !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
   ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
   DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
            DT_SPEC_AND_COMMA)
};


// add for joystick
int32_t preX = 0 , preY = 0;
static const int ADC_MAX = 1023;
// static const int ADC_MIN = 0;
static const int AXIS_DEVIATION = ADC_MAX / 2;
int32_t nowX = 0;
int32_t nowY = 0;

int start_index = 0;
int current_led_index = 0;
int cat_led_index = 55;
int previous_cat_led_index = 0;
int battery_level = 7;
int n = 10;
int *walls = NULL;
int walls_count = 0;


//rotary
#if !DT_NODE_EXISTS(DT_ALIAS(qdec0))
#error "Unsupported board: qdec0 devicetree alias is not defined"
#endif

#define SW_NODE DT_NODELABEL(gpiosw)
#if !DT_NODE_HAS_STATUS(SW_NODE, okay)
#error "Unsupported board: gpiosw devicetree alias is not defined or enabled"
#endif
static const struct gpio_dt_spec sw = GPIO_DT_SPEC_GET(SW_NODE, gpios);

static bool sw_led_flag = false;

static struct gpio_callback sw_cb_data;

void sw_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
   printk("SW pressed\n");
   sw_led_flag = !sw_led_flag;
}

// cat의 위치를 무작위로 초기화
void init_cat_position() {
    srand(k_uptime_get_32()); // 랜덤 시드 초기화
    int range = rand() % 2; // 0 또는 1의 무작위 수 생성

    if (range == 0) {
        cat_led_index = 52 + (rand() % 8); // 52~59 범위에서 무작위 선택
    } else {
        cat_led_index = 68 + (rand() % 8); // 68~75 범위에서 무작위 선택
    }
}

// 게임 상태를 초기화
void reset_game_state() {
    // Reset all relevant variables to their initial values
    start_index = 0;

    current_led_index = 0;
    init_cat_position(); // cat의 위치 초기화
    walls_count = 0; // user가 세운 벽의 개수 초기화
    if (walls != NULL) {
        free(walls);
        walls = NULL;
    }
    
    battery_level--;
    if(battery_level <= 0) {
        led_display_game_over();
        battery_level = 7;
        return;
   }

   led_init(start_index);
   add_wall(cat_led_index);
}

// cat이 가장자리에 닿으면(탈출하면)
void check_game_over(int index)
{
    if (index >= 0 && index <= 15) {
        printk("Game Over\n");
        k_sleep(K_MSEC(500));
      reset_game_state();
    } else if (index == 16 || index == 31 || index == 32 || index == 47 ||
               index == 48 || index == 63 || index == 64 || index == 79 ||
            index == 80 || index == 95 || index == 96 || 
            index == 111 ||(index >= 112 && index <= 127)) {
        printk("Game Over\n");
        k_sleep(K_MSEC(500));
      reset_game_state();
    }
}

/*
void move_cat() {
   remove_wall(cat_led_index);

   srand(k_uptime_get_32());
   int move_direction = rand() % 4; // 0부터 3까지의 무작위 수 생성

   if (move_direction == 0) {
      cat_led_index = led_on_right(cat_led_index);
   } else if(move_direction == 1) {
      cat_led_index = led_on_left(cat_led_index);
   } else if(move_direction == 2) {
      cat_led_index = led_on_up(cat_led_index);
   } else {
      cat_led_index = led_on_down(cat_led_index);
   }
   
   add_wall(cat_led_index);
   check_game_over(cat_led_index);
}
*/


// 다익스트라 알고리즘에 필요한 정의들
#define NUM_NODES 128
#define INF INT_MAX

int edges[NUM_NODES][NUM_NODES];
int dist[NUM_NODES];
int prev[NUM_NODES];
bool visited[NUM_NODES];

void init_graph() {
    for (int i = 0; i < NUM_NODES; i++) {
        for (int j = 0; j < NUM_NODES; j++) {
            edges[i][j] = INF;
        }
    }

    for (int i = 0; i < 128; i++) {
        if ((i + 1) % 16 != 0) edges[i][i + 1] = 1; // 오른쪽으로 이동 가능
        if (i % 16 != 0) edges[i][i - 1] = 1; // 왼쪽으로 이동 가능
        if (i + 16 < 128) edges[i][i + 16] = 1; // 아래로 이동 가능
        if (i - 16 >= 0) edges[i][i - 16] = 1; // 위로 이동 가능
    }

    for (int i = 0; i < walls_count; i++) {
        for (int j = 0; j < 128; j++) {
            edges[walls[i]][j] = INF;
            edges[j][walls[i]] = INF;
        }
    }
}

void dijkstra(int start) {
    for (int i = 0; i < NUM_NODES; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = false;
    }
    dist[start] = 0;

    for (int i = 0; i < NUM_NODES; i++) {
        int min_dist = INF;
        int u = -1;
        for (int j = 0; j < NUM_NODES; j++) {
            if (!visited[j] && dist[j] < min_dist) {
                min_dist = dist[j];
                u = j;
            }
        }

        if (u == -1) break;

        visited[u] = true;

        for (int v = 0; v < NUM_NODES; v++) {
            if (!visited[v] && edges[u][v] != INF && dist[u] + edges[u][v] < dist[v]) {
                dist[v] = dist[u] + edges[u][v];
                prev[v] = u;
            }
        }
    }
}

int get_next_move(int start, int end) {
    dijkstra(start);
    int u = end;
    while (prev[u] != start && prev[u] != -1) {
        u = prev[u];
    }
    return u;
}

void move_cat() {
    previous_cat_led_index = cat_led_index; // 이전 위치 저장

    remove_wall(cat_led_index);

    init_graph();

    int nearest_edge_index = -1;
    int min_distance = INF;

    // 가장자리 인덱스들
    int edge_indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
                          16, 31, 32, 47, 48, 63, 64, 79, 80, 95, 96, 111, 
                          112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127};

    dijkstra(cat_led_index);

    for (int i = 0; i < sizeof(edge_indices)/sizeof(edge_indices[0]); i++) {
        if (dist[edge_indices[i]] < min_distance) {
            min_distance = dist[edge_indices[i]];
            nearest_edge_index = edge_indices[i];
        }
    }

    if (nearest_edge_index != -1) {
        int next_move = get_next_move(cat_led_index, nearest_edge_index);
        // 벽이 있는 위치로 이동하지 않도록 확인
        bool is_wall = false;
        for (int i = 0; i < walls_count; i++) {
            if (walls[i] == next_move) {
                is_wall = true;
                break;
            }
        }

        if (!is_wall) {
            cat_led_index = next_move;
        }
    }

    add_wall(cat_led_index);

    if (cat_led_index == previous_cat_led_index) {
        game_clear();
    } else {
        check_game_over(cat_led_index);
    }
}



void game_clear() {
    printk("Game Clear!\n");
    led_display_game_clear();
    k_sleep(K_MSEC(2000));
    battery_level = 7;
    start_index = 0;
    current_led_index = 0;
    init_cat_position(); // cat의 위치 초기화
    walls_count = 0; // user가 세운 벽의 개수 초기화
    if (walls != NULL) {
        free(walls);
        walls = NULL;
    }
}

bool is_wall_at(int index, int *walls, int walls_count) {
    for (int i = 0; i < walls_count; i++) {
        if (walls[i] == index) {
            return true;
        }
    }
    return false;
}

void display_rotary_led(int32_t rotary_val)
{
	if (rotary_val >30) { //오른쪽으로 회전
		led_off_without_walls(walls, walls_count);
		add_wall(current_led_index);
		move_cat(); // Move the cat to the right
		//cat_led_index = led_on_right(cat_led_index);
	} else if (rotary_val < -20) { //왼쪽으로 회전
        int random_wall_indices[2];
        for (int i = 0; i < 2; i++) {
            bool unique;
            do {
                unique = true;
                random_wall_indices[i] = rand() % 128; // 0부터 127 사이의 무작위 위치 선택
                for (int j = 0; j < i; j++) {
                    if (random_wall_indices[i] == random_wall_indices[j] || 
                        is_wall_at(random_wall_indices[i], walls, walls_count) ||
                        random_wall_indices[i] == cat_led_index) {
                        unique = false;
                        break;
                    }
                }
            } while (!unique);

            add_wall(random_wall_indices[i]);
        }
	}
}


bool isChange(void)
{
	if((nowX < (preX - 50)) || nowX > (preX+50)){
		preX = nowX;
		return true;
	}

	if((nowY < (preY - 50)) || nowY > (preY+50)){
		preY = nowY;
		return true;
	}
	return false;
}

void add_wall(int index)
{
   if (walls == NULL)
   {
      walls = (int *)malloc(n * sizeof(int));
   }
   if (walls_count >= n)
   {
      n *= 2;
      walls = (int *)realloc(walls, n * sizeof(int));
   }
   printk("%d", walls_count);
   walls[walls_count++] = index;
   led_off_without_walls(walls, walls_count);
}

void remove_wall(int index)
{
    // 벽이 존재하는지 확인
    if (walls != NULL && walls_count > 0) {
        // 벽을 찾아서 제거
        for (int i = 0; i < walls_count; i++) {
            if (walls[i] == index) {
                // 벽을 찾았으면 해당 위치의 LED를 끔
                led_off(led, index);
                // 벽을 제거하고 배열을 재정렬
                for (int j = i; j < walls_count - 1; j++) {
                    walls[j] = walls[j + 1];
                }
                // 벽 카운트를 감소시킴
                walls_count--;
                break;
            }
        }
    }
}

void process_joystick_input()
{
    uint16_t buf;
    int err;
    struct adc_sequence sequence = {
        .buffer = &buf,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(buf),
    };
    
    while(1) {
        (void)adc_sequence_init_dt(&adc_channels[0], &sequence);
        err = adc_read(adc_channels[0].dev, &sequence);
        if (err < 0) {
            printk("Could not read ADC channel 0 (%d)\n", err);
            return;
        }

        nowX = (int32_t)buf;

        (void)adc_sequence_init_dt(&adc_channels[1], &sequence);
        err = adc_read(adc_channels[1].dev, &sequence);
        if (err < 0) {
            printk("Could not read ADC channel 1 (%d)\n", err);
            return;
        }

        nowY = (int32_t)buf;

        printk("Joy X: %" PRIu32 ", Joy Y: %" PRIu32 "\n", nowX, nowY);

        if (nowX >= 65500 || nowY >= 65500){
            printk("Out of Range\n");
            // return;
        }

        bool checkFlag = isChange();
        if(!checkFlag){
            printk("No Change\n");
            return;
        } 

        if (nowX == ADC_MAX && nowY == ADC_MAX){
            printk("Center\n");
        } else if ((nowX < AXIS_DEVIATION || nowX >= 65500) && nowY == ADC_MAX){ // 조이스틱 왼쪽으로
            led_off_without_walls(walls, walls_count);
            current_led_index = led_on_left(current_led_index);
            printk("Left\n");
        } else if (nowX > AXIS_DEVIATION && nowY == ADC_MAX){ // 조이스틱 오른쪽으로
            led_off_without_walls(walls, walls_count);
            current_led_index = led_on_right(current_led_index);
            //add_wall(current_led_index);
            printk("Right\n");
        } else if ((nowY >= 65500 || nowY < AXIS_DEVIATION) && nowX == ADC_MAX){ // 조이스틱 아래로
            led_off_without_walls(walls, walls_count);
            current_led_index = led_on_down(current_led_index);
            printk("Down\n");
		} else if (nowY > AXIS_DEVIATION && nowX == ADC_MAX){ // 조이스틱 위로
            led_off_without_walls(walls, walls_count);
            current_led_index = led_on_up(current_led_index);
            printk("Up\n");
        }

        k_sleep(K_MSEC(100));
    }
}

int main(void)
{
   init_cat_position();
   //rotary
   struct sensor_value val;
   int rc;
   const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(qdec0));

   if (!device_is_ready(dev)) {
      printk("Qdec device is not ready\n");
      return 0;
   }

   if(!gpio_is_ready_dt(&sw)) {
      printk("SW GPIO is not ready\n");
      return 0;
   }

   int err = gpio_pin_configure_dt(&sw, GPIO_INPUT);
   if(err < 0){
      printk("Error configuring SW GPIO pin %d\n", err);
      return 0;
   }

   err = gpio_pin_interrupt_configure_dt(&sw, GPIO_INT_EDGE_RISING);
   if(err != 0){
      printk("Error configuring SW GPIO interrupt %d\n", err);
      return 0;
   }

   err = batterydisplay_intit();
   if(err != 0){
         printk("Error initializing battery display\n");
         return DK_ERR;
   }

   gpio_init_callback(&sw_cb_data, sw_callback, BIT(sw.pin));
   gpio_add_callback(sw.port, &sw_cb_data);

   printk("Quadrature decoder sensor test\n");
   
   // ADC 채널 설정
   for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
      if (!adc_is_ready_dt(&adc_channels[i])) {
         printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
         return 0;
      }

      err = adc_channel_setup_dt(&adc_channels[i]);
      if (err < 0) {
         printk("Could not setup channel #%d (%d)\n", i, err);
         return 0;
      }
   }

   add_wall(cat_led_index);
   led_init(start_index);

   while (1) {
      display_level(battery_level);

      rc = sensor_sample_fetch(dev);
      if (rc != 0) {
         printk("Failed to fetch sample (%d)\n", rc);
         return 0;
      }

      rc = sensor_channel_get(dev, SENSOR_CHAN_ROTATION, &val);
      if (rc != 0) {
         printk("Failed to get data (%d)\n", rc);
         return 0;
      }
      
      if(!sw_led_flag){
         display_rotary_led(val.val1);
      } 

      printk("Position = %d degrees \n", val.val1);

      k_msleep(100);
      
      process_joystick_input();

      k_msleep(100);
   }
   free(walls);
   return 0;
}