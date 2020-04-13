#include <motors.h>

#define DESIRED_SPEED 	   (int)(0.3 * (float)MOTOR_SPEED_LIMIT)
#define WHEEL_PERIMETER     13 // [cm]

enum direction {
	forward, backward, left, right
};

void set_direction(enum direction direction);
void turn_left(uint16_t angle);
void turn_right(uint16_t angle);
