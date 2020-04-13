#include "ch.h"
#include "hal.h"

#include <main.h>
#include <find_path_to_exit.h>
#include <manage_motors.h>
#include <sensors/proximity.h>
#include <math.h>
#include <stdbool.h>
#include <motors.h> //needed for the define of MOTOR_SPEED_LIMIT

#include <chprintf.h> //needed for the
#include <usbcfg.h>	 //chprintf



#define DISTANCE_DESCRIBING_NO_RIGHT_WALL 120 // very small value because no wall means that proximity sensors returns LOW values (cf : graph value sensors)
#define DISTANCE_DESCRIBING_FORWARD_WALL 250 //In theory should be approx 760 for 1 cm distance (cf: graph value sensors). In pratic 300 is nice
#define DISTANCE_DESCRIBING_LEFT_WALL 90 //In theory should be approx 760 for 1 cm distance (cf: graph value sensors). In pratic 300 is nice
#define DIAMETER_ROBOT	7.5f
#define LABIRYNTHE_CELL_SIZE	9.5f //The labirynthe is composed of equals cell sizes
#define MAX_SUM_ERROR (MOTOR_SPEED_LIMIT/KI)
#define ERROR_THRESHOLD 150
#define PROXIMITY_DISTANCE_GOAL 1000
#define KP  800
#define KI	3.5
#define ANGLE_CORRECTION	2

static uint16_t time_to_wait_after_turned = (uint16_t)(1.1 * 1000.0 * (LABIRYNTHE_CELL_SIZE/2.0)/((float)(DESIRED_SPEED) * (float)(WHEEL_PERIMETER) / (float)(MOTOR_SPEED_LIMIT))); //t = d/v //*1000 because its ms //*1.1 to include 10% marge errors because labirynthe cells are not exactly 9.5 cm
static systime_t time2;
static uint64_t distance_corrected_backward = 0;
static uint64_t distance_traveled_after_detecting_solution_on_the_right = 0;
static bool goback = false;
static bool pi_regulator_is_on = true;
static int counter_frequence = 0;

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


void process_pi_regulator(void){
	 enum distance distance = perfect;


    //computes if the robot is too close, too far away or at a good distance of the wall
    distance = pi_regulator(get_prox(RIGHT), PROXIMITY_DISTANCE_GOAL);

    if(distance == too_close && pi_regulator_is_on == true) {
    		//turn robot a little bit on the left if robot is too close from right wall
    		turn_left((uint16_t)ANGLE_CORRECTION);
    		set_direction(forward);
    } else if(distance == too_far && pi_regulator_is_on == true) {
    		//turn robot a little bit on the right if robot is too far from right wall
    		turn_right((uint16_t)ANGLE_CORRECTION);
    		set_direction(forward);
    }
}

void check_for_path(void) {
	//define and compute distance of walls using proximity sensors
	int right_distance = get_prox(RIGHT);
	int forward_distance = get_prox(FRONT_RIGHT17);
	int left_distance = get_prox(LEFT);

	//Check if robot has to go backward because he find opportunity to turn to the right
	if(goback == true) {
		//Need to disable the pi_regulator
		pi_regulator_is_on = false;

		//Check if distance done in backward is enough to turn right or robot need to continue go backward
		if(distance_corrected_backward > distance_traveled_after_detecting_solution_on_the_right/2){
			set_direction(backward);
			distance_corrected_backward --;
			return;
		} else {
			//RESET the distance for backwarding and the statement to go backward to false
			distance_corrected_backward = 0;
			distance_traveled_after_detecting_solution_on_the_right = 0;
			goback = false;

			//The robot has to turn immediately and then go forward
			turn_right((uint16_t)90);
			set_direction(forward);

			//Wait a little so that the robot finished to turn himself and start a bit to go forward before checking if there is walls again
			time2 = chVTGetSystemTime();
			chThdSleepUntilWindowed(time2, time2 + MS2ST(time_to_wait_after_turned));

			//Need to able the pi_regulator
			pi_regulator_is_on = true;
		}
	}


	/*Logic to solve the labyrinthe : -turn to the right if there is wall in front and nothing at right
	 * 								  -turn to the right if there is no right wall
	 * 					              -continue forward if there is right wall and no forward wall
	 * 					              -turn to the left is there is right wall and forward wall and no left wall
	 * 					              -turn around else
	 */
	if(right_distance < DISTANCE_DESCRIBING_NO_RIGHT_WALL && forward_distance > DISTANCE_DESCRIBING_FORWARD_WALL) {
		//Need to disable the pi_regulator
		pi_regulator_is_on = false;

		//RESET the distance for backwarding
		distance_traveled_after_detecting_solution_on_the_right = 0;
		distance_corrected_backward = 0;

		//The robot has to turn immediately and then go forward
		turn_right((uint16_t)90);
		set_direction(forward);

		//Wait a little so that the robot finished to turn himself and start a bit to go forward before checking if there is walls again
		time2 = chVTGetSystemTime();
		chThdSleepUntilWindowed(time2, time2 + MS2ST(time_to_wait_after_turned));

		//Need to able the pi_regulator
		pi_regulator_is_on = true;
	} else if(right_distance < DISTANCE_DESCRIBING_NO_RIGHT_WALL && forward_distance < DISTANCE_DESCRIBING_FORWARD_WALL) {
		//Need to disable the pi_regulator
		pi_regulator_is_on = false;

		//There is no right wall so we can increment our distance to correct with backwarding
		distance_traveled_after_detecting_solution_on_the_right ++;
		distance_corrected_backward ++;
	} else if(right_distance > DISTANCE_DESCRIBING_NO_RIGHT_WALL && forward_distance < DISTANCE_DESCRIBING_FORWARD_WALL) {
		//Need to able the pi_regulator
		pi_regulator_is_on = true;

		set_direction(forward);

		//Check is if the robot just finished to see an opportunity to go to the right
		if(distance_traveled_after_detecting_solution_on_the_right > 10){ //10 = treshold ?
			//Need to disable the pi_regulator
			pi_regulator_is_on = false;

			goback = true;
			set_direction(backward);
		}
	} else if(right_distance > DISTANCE_DESCRIBING_NO_RIGHT_WALL && forward_distance > DISTANCE_DESCRIBING_FORWARD_WALL && left_distance < DISTANCE_DESCRIBING_LEFT_WALL) {
		//Need to disable the pi_regulator
		pi_regulator_is_on = false;

		//RESET the distance for backwarding
		distance_traveled_after_detecting_solution_on_the_right = 0;
		distance_corrected_backward = 0;

		//The robot has to turn immediately and then go forward
		turn_left((uint16_t)90);
		set_direction(forward);

		//Wait a little so that the robot finished to turn himself and start a bit to go forward before checking if there is walls again
		time2 = chVTGetSystemTime();
		chThdSleepUntilWindowed(time2, time2 + MS2ST(time_to_wait_after_turned));

		//Need to able the pi_regulator
		pi_regulator_is_on = true;
	} else {
		//Need to disable the pi_regulator
		pi_regulator_is_on = false;

		//RESET the distance for backwarding
		distance_traveled_after_detecting_solution_on_the_right = 0;
		distance_corrected_backward = 0;

		//The robot has to turn around immediately and then go forward
		turn_right((uint16_t)180);
		set_direction(forward);

		//Wait 1 seconds so that the robot finished to turn himself and start a bit to go forward before checking if there is walls
		time2 = chVTGetSystemTime();
		chThdSleepUntilWindowed(time2, time2 + MS2ST(time_to_wait_after_turned));

		//Need to able the pi_regulator
		pi_regulator_is_on = true;
	}
}



static THD_WORKING_AREA(waFindPathToExit, 512);
static THD_FUNCTION(FindPathToExit, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    while(1){
        time = chVTGetSystemTime();

        if(pi_regulator_is_on == true && counter_frequence == 5){
        		process_pi_regulator();
        }

        check_for_path();

        //Define frequency of PI regulator to 2Hz
        if(counter_frequence == 5) {
        		counter_frequence = 0;
        } else {
            counter_frequence++;
        }


        //10Hz
        chThdSleepUntilWindowed(time, time + MS2ST(100));
    }
}

void find_path_to_exit_start(void){
	chThdCreateStatic(waFindPathToExit, sizeof(waFindPathToExit), NORMALPRIO + 1, FindPathToExit, NULL);
}
