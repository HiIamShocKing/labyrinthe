#include "msgbus/messagebus.h"
#include "camera/dcmi_camera.h"
//#include "parameter/parameter.h"
#include <audio/play_melody.h>
#include <audio/audio_thread.h>

#define FRONT_RIGHT17	0
#define FRONT_RIGHT49 	1
#define RIGHT 			2
#define BACK_RIGHT		3
#define BACK_LEFT 		4
#define LEFT 			5
#define FRONT_LEFT49 	6
#define FRONT_LEFT17 	7

extern messagebus_t bus;

typedef enum{
	slow = 2000,
	normal = 1000,
	fast = 500
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

