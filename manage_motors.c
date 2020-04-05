#include <motors.h>
#include <manage_motors.h>

#define NSTEP_ONE_TURN      1000 // number of step for 1 turn of the motor
#define WHEEL_PERIMETER     13 // [cm]
#define PI                  3.1415926536f

//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)

//Turn the robot to the left of a given angle in degrees between 0 et 360
void turn_left(uint16_t angle) {
	//Put the pos counter to 0
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	//Set speeds of motor
	left_motor_set_speed(-0.3*MOTOR_SPEED_LIMIT);
    right_motor_set_speed(0.3*MOTOR_SPEED_LIMIT);

    //Check if position is reached for the given angle and then stop motors
    int32_t left_pos = left_motor_get_pos();
    int32_t right_pos = right_motor_get_pos();
    int32_t position_to_reach_left = (int32_t)angle/90 * PERIMETER_EPUCK/4 * NSTEP_ONE_TURN / WHEEL_PERIMETER;
    int32_t position_to_reach_right = (int32_t)angle/90 * PERIMETER_EPUCK/4 * NSTEP_ONE_TURN / WHEEL_PERIMETER;
    while (left_pos < position_to_reach_left && right_pos < position_to_reach_right) {
    		left_pos = left_motor_get_pos();
    		right_pos = right_motor_get_pos();
    }
    left_motor_set_speed(0);
    right_motor_set_speed(0);

}

//Turn the robot to the left of a given angle in degrees between 0 et 360
void turn_right(uint16_t angle) {
	//Put the pos counter to 0
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	//Set speeds of motor
	left_motor_set_speed(0.3*MOTOR_SPEED_LIMIT);
    right_motor_set_speed(-0.3*MOTOR_SPEED_LIMIT);

    //Check if position is reached for the given angle and then stop motors
    int32_t left_pos = left_motor_get_pos();
    int32_t right_pos = right_motor_get_pos();
    int32_t position_to_reach_left = (int32_t)angle/90 * PERIMETER_EPUCK/4 * NSTEP_ONE_TURN / WHEEL_PERIMETER;
    int32_t position_to_reach_right = (int32_t)angle/90 * PERIMETER_EPUCK/4 * NSTEP_ONE_TURN / WHEEL_PERIMETER;
    while (left_pos < position_to_reach_left && right_pos < position_to_reach_right) {
    		left_pos = left_motor_get_pos();
    		right_pos = right_motor_get_pos();
    }
    left_motor_set_speed(0);
    right_motor_set_speed(0);

}

void set_direction(enum direction direction) {
	switch(direction) {
	case forward:
		left_motor_set_speed(MOTOR_SPEED_LIMIT);
	    right_motor_set_speed(MOTOR_SPEED_LIMIT);
		break;
	case backward:
		left_motor_set_speed(-MOTOR_SPEED_LIMIT);
	    right_motor_set_speed(-MOTOR_SPEED_LIMIT);
		break;
	case left:
		turn_left(90);
		break;
	case right:
		turn_right(90);
		break;
	}
}
