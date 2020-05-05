#include <motors.h>

#define NUMBER_STEPS_FOR_ONE_TURN      1000 // number of step for 1 wheel turn of the motor
#define PI                  3.1415926536f
#define WHEEL_PERIMETER     13 // [cm]
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)

typedef enum{
	forward, backward, left, right
}direction;

void set_direction(direction direction);
void turn_left(uint16_t angle);
void turn_right(uint16_t angle);
void stop_motors(void);
