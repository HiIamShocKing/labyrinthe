#include <manage_motors.h>
#include <chprintf.h>
#include <usbcfg.h>

#define NSTEP_ONE_TURN      1000 // number of step for 1 turn of the motor
#define PI                  3.1415926536f


//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)


/**
* @brief   Turn the robot to the left of a given angle in degrees >= 0
*
* @param angle		the angle is degrees we want the robot to turn
*/
void turn_left(uint16_t angle) {
	//Return of the function if we don't want to turn the robot
	if(angle == 0) {
		return;
	}

	//Put the position counter to 0
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	//Set speeds of motor
	left_motor_set_speed(-DESIRED_SPEED);
    right_motor_set_speed(DESIRED_SPEED);

    //Check if position is reached for the given angle and then stop motors
    int32_t left_pos = left_motor_get_pos();
    int32_t right_pos = right_motor_get_pos();
    int32_t pos_to_reach_left = (int32_t)((float)angle/90.0 * PERIMETER_EPUCK/4.0 * (float)NSTEP_ONE_TURN / (float)WHEEL_PERIMETER);
    int32_t pos_to_reach_right = (int32_t)((float)angle/90.0 * PERIMETER_EPUCK/4.0 * (float)NSTEP_ONE_TURN / (float)WHEEL_PERIMETER);
    while (left_pos < pos_to_reach_left && right_pos < pos_to_reach_right) {
    		left_pos = left_motor_get_pos();
    		right_pos = right_motor_get_pos();
    }
    left_motor_set_speed(0);
    right_motor_set_speed(0);
}

/**
* @brief   Turn the robot to the right of a given angle in degrees >= 0
*
* @param angle		the angle is degrees we want the robot to turn
*/
void turn_right(uint16_t angle) {
	//Return of the function if we don't want to turn the robot
	if(angle == 0) {
		return;
	}

	//Put the pos counter to 0
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	//Set speeds of motor
	left_motor_set_speed(DESIRED_SPEED);
    right_motor_set_speed(-DESIRED_SPEED);

    //Check if position is reached for the given angle and then stop motors
    int32_t left_pos = left_motor_get_pos();
    int32_t right_pos = right_motor_get_pos();
    int32_t pos_to_reach_left = (int32_t)((float)angle/90.0 * PERIMETER_EPUCK/4.0 * (float)NSTEP_ONE_TURN / (float)WHEEL_PERIMETER);
    int32_t pos_to_reach_right = (int32_t)((float)angle/90.0 * PERIMETER_EPUCK/4.0 * (float)NSTEP_ONE_TURN / (float)WHEEL_PERIMETER);
    while (left_pos < pos_to_reach_left && right_pos < pos_to_reach_right) {
    		left_pos = left_motor_get_pos();
    		right_pos = right_motor_get_pos();
    }
    left_motor_set_speed(0);
    right_motor_set_speed(0);

}

/**
* @brief   Set the direction of the robot wanted
*
* @param enum direction	case forward : Robot moves forward
* 						case backward : Robot moves backward
* 						case right : Robot turns 90° to the right then stop
* 						case left : Robot turns 90° to the left then stop
*/
void set_direction(enum direction direction) {
	switch(direction) {
	case forward:
		left_motor_set_speed(DESIRED_SPEED);
	    right_motor_set_speed(DESIRED_SPEED);
		break;
	case backward:
		left_motor_set_speed(-DESIRED_SPEED);
	    right_motor_set_speed(-DESIRED_SPEED);
		break;
	case left:
		turn_left((uint16_t)90);
		break;
	case right:
		turn_right((uint16_t)90);
		break;
	}
}
