//#include "ch.h"
//#include "hal.h"
#include <main.h>
#include <find_path_to_exit.h>
#include <manage_motors.h>
#include <sensors/proximity.h>
#include <math.h>
#include <stdbool.h>

#include <chprintf.h> //needed for the
#include <usbcfg.h>	 //chprintf

static systime_t time;
static uint16_t distance_corrected_backward = 0;
static uint16_t right_path_thickness = 0;
static uint8_t counter_pi_frequence = 0;
static uint8_t counter_pi_frequence_to_reach = 1;
static bool pi_regulator_is_on = true;
static bool go_backward = false;

extern int desired_speed;
//extern bool detect_peach;
//extern toggle_speed toggle_period_rgb_led;
//extern bool is_turning;

/**
* @brief   PI regulator that checks if the robot is too far or too close of his right wall
*
* @param distance		measured distance value given by proximity sensors
* 		goal				proximity value distance goal of the wall
*
* @return				too_close, too_far or perfect
*/
distance pi_regulator(int distance, int goal) {
	int16_t error = 0;
	static int16_t sum_error = 0;

	//The sensor gives high values for small distance and low values for big distance
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

/**
* @brief  Wait before checking anything more that the robot did move enough after turned
*
* @param time_to_wait_after_turned		time needed to wait
*/
void wait_after_turned(uint16_t time_to_wait_after_turned){
	time = chVTGetSystemTime();
	chThdSleepUntilWindowed(time, time + MS2ST(time_to_wait_after_turned));
}

/**
* @brief   Do the PI regulation
*/
void process_pi_regulator(void){
	 distance distance = perfect;

    //computes if the robot is too close, too far away or at a good distance of the wall
    distance = pi_regulator(get_prox(RIGHT), PROXIMITY_DISTANCE_GOAL);

    if(distance == too_close && pi_regulator_is_on == true) {
       	turn_left((uint16_t)ANGLE_CORRECTION);
        	set_direction(forward);
    } else if(distance == too_far && pi_regulator_is_on == true) {
    		turn_right((uint16_t)ANGLE_CORRECTION);
    		set_direction(forward);
    }
}

/**
* @brief   Go backward and update distance counter
*/
void backward_correction(void){
	set_direction(backward);
	distance_corrected_backward ++;
}

/**
* @brief  Do the backward correction, reset all variables linked to it
*/
void turn_right_after_backward_correction(void){
	//The robot has to turn immediately and then go forward
	set_is_turning(true);
	turn_right((uint16_t)90);
	set_direction(forward);
	set_is_turning(false);
}

/**
* @brief   reset variables linked to backward
*/
void reset_counters_backwarding(void){
	distance_corrected_backward = 0;
	right_path_thickness = 0;
}

/**
* @brief   move the robot into the labyrinth to get out
*/
void check_for_path(void) {
	//define and compute proximity value distance of walls using proximity sensors
	uint16_t right_distance = get_prox(RIGHT);
	uint16_t forward_distance = get_prox(FRONT_RIGHT17);
	uint16_t left_distance = get_prox(LEFT);

	//define the time to wait to check again walls after turned
	//wait the time so that the robot finished to get out of the actually cell in the labyrinth
	//t = d/v //*1000 to convert in ms
			  //*1.1 to include 10% possible errors because labirynthe's size cells are not all perfect identical
			  //d = LABIRYNTHE_CELL_SIZE/2.0 [cm] which is the half of the width of the path
			  //v is given by desired_speed[steps/s] converted to [cm/s]
	uint16_t time_to_wait_after_turned = (uint16_t)(1.1 * 1000.0 * (LABIRYNTHE_CELL_SIZE/2.0)/
			((float)(desired_speed) * (float)(WHEEL_PERIMETER) / (float)(NUMBER_STEPS_FOR_ONE_TURN)));

	/*Global Logic to solve the labyrinthe : always turn to right if possible.
	 *
	 *	-go backward if right wall thickness is enough high
	 *	-turn to the right if there is wall in front and nothing at right
	 *	-continue forward if there is no right wall and no forward wall and compute thickness of the right path
	 *	-tells to go backward if right wall thickness is enough high to then go to the right path detected before
	 *	-turn to the left if there right wall and front wall and nothing at the left
	 *	-turn around if there is right wall, front wall and left wall
	 */
	if(go_backward == true){
		//Robot has to go backward to meet the center axis of the right path and then turn
		if(distance_corrected_backward <= right_path_thickness/2){
			backward_correction();
		} else{
			turn_right_after_backward_correction();
			wait_after_turned(time_to_wait_after_turned);

			pi_regulator_is_on = true;
			reset_counters_backwarding();
			go_backward = false;
		}
	} else if(right_distance < DISTANCE_DESCRIBING_NO_RIGHT_WALL && forward_distance > DISTANCE_DESCRIBING_FORWARD_WALL){
		reset_counters_backwarding();

		//The robot has to turn immediately and then go forward
		set_is_turning(true);
		turn_right((uint16_t)90);
		set_direction(forward);
		set_is_turning(false);

		wait_after_turned(time_to_wait_after_turned);

		/*Need to able back the pi_regulator because
		 * we managed to turn to the right with backwarding
		 * but we did it without backwarding*/
		pi_regulator_is_on = true;
	} else if(right_distance < DISTANCE_DESCRIBING_NO_RIGHT_WALL && forward_distance < DISTANCE_DESCRIBING_FORWARD_WALL){
		//Need to disable the pi_regulator because we manage to turn to the right
		pi_regulator_is_on = false;

		//There is no right wall so compute thickness of the right path
		right_path_thickness ++;
	} else if(right_distance > DISTANCE_DESCRIBING_NO_RIGHT_WALL && forward_distance < DISTANCE_DESCRIBING_FORWARD_WALL) {
		if(right_path_thickness > THRESHOLD_RIGHT_WALL_THICKNESS){
			go_backward = true;
		}
	} else if(right_distance > DISTANCE_DESCRIBING_NO_RIGHT_WALL && forward_distance > DISTANCE_DESCRIBING_FORWARD_WALL && left_distance < DISTANCE_DESCRIBING_LEFT_WALL) {
		reset_counters_backwarding();

		//The robot has to turn immediately and then go forward
		set_is_turning(true);
		turn_left((uint16_t)90);
		set_direction(forward);
		set_is_turning(false);

		wait_after_turned(time_to_wait_after_turned);
	} else {
		reset_counters_backwarding();

		//The robot has to turn around immediately and then go forward
		set_is_turning(true);
		turn_left((uint16_t)180);
		set_direction(forward);
		set_is_turning(false);

		wait_after_turned(time_to_wait_after_turned);
	}
}

/**
* @brief   Determine counter_pi_frequency to reach
*/
void determine_pi_frequency(void){
    switch(get_toggle_period_rgb_led()){
    case(normal):
    		//frequency for pi regulator should be 2hz = 20hz/10 so we want the counter to be 10
    		if(counter_pi_frequence_to_reach != FREQUENCY_THREAD_FIND_PATH_TO_EXIT/2){
    			counter_pi_frequence = 0;
    			counter_pi_frequence_to_reach = FREQUENCY_THREAD_FIND_PATH_TO_EXIT/2;
    		}
    		break;
    case(fast):
		//frequency for pi regulator should be 5hz = 20hz/4 so we want the counter to be 4
         if(counter_pi_frequence_to_reach != FREQUENCY_THREAD_FIND_PATH_TO_EXIT/5){
           	counter_pi_frequence = 0;
           	counter_pi_frequence_to_reach = FREQUENCY_THREAD_FIND_PATH_TO_EXIT/5;
         }
    		break;
    case(slow):
    		//frequency for pi regulator should be 1hz = 20hz/20 so we want the counter to be 20
        if(counter_pi_frequence_to_reach != FREQUENCY_THREAD_FIND_PATH_TO_EXIT){
            counter_pi_frequence = 0;
			    counter_pi_frequence_to_reach = FREQUENCY_THREAD_FIND_PATH_TO_EXIT;
         }
    		break;
    }
}

static THD_WORKING_AREA(waFindPathToExit, 1024);
static THD_FUNCTION(FindPathToExit, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    while(1){
    		//robot detect princess peach so that it means he got out of the labyrinth
    		//so we disable this thread
    		if(get_detect_peach() == true){
    			while(1){
    				chThdSleepMilliseconds(PERIOD_SLEEP_THREAD_FIND_PATH_TO_EXIT);
    			}
    		}

        time = chVTGetSystemTime();

        //logic function/move robot to find the exit of the labyrinth
        check_for_path();

        //Do PI regulation at right frequency
        if(pi_regulator_is_on == true && counter_pi_frequence == counter_pi_frequence_to_reach){
        		process_pi_regulator();
        }

        determine_pi_frequency();

        if(counter_pi_frequence == counter_pi_frequence_to_reach){
        		counter_pi_frequence = 0;
        }else{
        		counter_pi_frequence++;
        }

        //20Hz
        chThdSleepUntilWindowed(time, time + MS2ST(PERIOD_SLEEP_THREAD_FIND_PATH_TO_EXIT));
    }
}

void find_path_to_exit_start(void){
	chThdCreateStatic(waFindPathToExit, sizeof(waFindPathToExit), NORMALPRIO - 1, FindPathToExit, NULL);
}
