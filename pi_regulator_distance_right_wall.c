#include "ch.h"
#include "hal.h"

#include <pi_regulator_distance_right_wall.h>
#include <main.h>
#include <manage_motors.h>
#include <sensors/proximity.h>
#include <math.h>
#include <chprintf.h> //needed for the
#include <usbcfg.h>	 //chprintf
#include <motors.h> //needed for the define of MOTOR_SPEED_LIMIT

#define MAX_SUM_ERROR (MOTOR_SPEED_LIMIT/KI)
#define ERROR_THRESHOLD 400
#define PROXIMITY_DISTANCE_GOAL 760
#define KP  800
#define KI	3.5

enum distance{
	too_close, too_far, perfect
};

/**
* @brief   PI regulator that checks if the robot is too far or too close of his right wall
*
* @param distance		measured distance by proximity sensors
* 		goal				proximity distance goal of the wall
*
* @return				too_close, too_far or perfect
*/
enum direction pi_regulator(int distance, int goal) {
	int error = 0;
	static int sum_error = 0;

	error = distance - goal;

	//disables the PI regulator if the error is to small
	if(fabs(error) < ERROR_THRESHOLD){
		return perfect;
	}

	sum_error += error;

	//we set a maximum and a minimum for the sum to avoid an uncontrolled growth
	if(sum_error > MAX_SUM_ERROR){
		sum_error = MAX_SUM_ERROR;
	}else if(sum_error < -MAX_SUM_ERROR){
		sum_error = -MAX_SUM_ERROR;
	}

	if((KP * error + KI * sum_error) > 0) {
		return too_close;
	} else if((KP * error + KI * sum_error) < 0) {
		return too_far;
	} else {
		return perfect;
	}
}

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    enum distance distance = perfect;

    while(1){
        time = chVTGetSystemTime();

        //computes if the robot is too close or too far away from the wall
        distance = pi_regulator(get_prox(RIGHT), PROXIMITY_DISTANCE_GOAL);

        if(distance == too_close) {
        		//turn robot on the left if robot is too close from right wall
        		turn_left((uint16_t)4);
        		set_direction(forward);
        } else if(distance == too_far) {
        		//turn robot on the right if robot is too far from right wall
        		turn_right((uint16_t)4);
        		set_direction(forward);
        }

        //2Hz
        chThdSleepUntilWindowed(time, time + MS2ST(500));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}
