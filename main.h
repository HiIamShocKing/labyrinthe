#include "msgbus/messagebus.h"
#include "camera/dcmi_camera.h"
#include <audio/play_melody.h>
#include <audio/audio_thread.h>

#define FRONT_RIGHT17	0
#define RIGHT 			2
#define LEFT 			5

#define QUARTER_TURN	(uint16_t)90
#define HALF_TURN	(uint16_t)180

#define NORMAL_TOGGLE_PERIOD	1000
#define SLOW_TOGGLE_PERIOD	2000
#define FAST_TOGGLE_PERIOD	500

extern messagebus_t bus;

typedef enum{
	slow = SLOW_TOGGLE_PERIOD,
	normal = NORMAL_TOGGLE_PERIOD,
	fast = FAST_TOGGLE_PERIOD
}toggle_speed;

typedef enum{
	white, yellow, blue, pink, green
}rgb_led_color;

void toggle_body_led(void);
bool get_detect_peach(void);
void set_detect_peach(bool boolean);
toggle_speed get_toggle_period_rgb_led(void);
void set_toggle_period_rgb_led(toggle_speed state);
rgb_led_color get_led_color(void);
void set_led_color(rgb_led_color color);
bool get_is_turning(void);
void set_is_turning(bool boolean);
void turn_off_led(void);
bool get_leds_statue(void);
void set_leds_statue(bool boolean);

