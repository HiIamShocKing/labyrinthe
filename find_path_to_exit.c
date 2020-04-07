#include "ch.h"
#include "hal.h"

#include <main.h>
#include <find_path_to_exit.h>
#include <manage_motors.h>
#include <sensors/proximity.h>

#include <chprintf.h> //needed for the
#include <usbcfg.h>	 //chprintf

#define DISTANCE_DESCRIBING_NO_WALL 70
//static int counter = 0;
void check_for_path(void) {
	//define and compute distance of walls using proximity sensors
	int right_distance = get_prox(RIGHT);
	int forward_distance = get_prox(FRONT_RIGHT17);
	int left_distance = get_prox(LEFT);

	/*Logic to solve the labyrinthe : -turn to the right if there is no right wall
	 * 					              -continue forward if there is right wall and no forward wall
	 * 					              -turn to the left is there is right wall and forward wall and no left wall
	 * 					              -turn around else
	 */
	systime_t time1;
	systime_t time2;
	if(right_distance < DISTANCE_DESCRIBING_NO_WALL) {
		//wait 1 second so that the entire robot can be able to go to the right
		time1 = chVTGetSystemTime();
		chThdSleepUntilWindowed(time1, time1 + MS2ST(1000));

		turn_right((uint16_t)90);

		//Wait 2 seconds so that the robot finished to turn himself and start a bit to go forward before checking if there is walls
		time2 = chVTGetSystemTime();
		chThdSleepUntilWindowed(time2, time2 + MS2ST(2000));
	} else if(right_distance > DISTANCE_DESCRIBING_NO_WALL && forward_distance < DISTANCE_DESCRIBING_NO_WALL) {
		set_direction(forward);
	} else if(right_distance > DISTANCE_DESCRIBING_NO_WALL && forward_distance > DISTANCE_DESCRIBING_NO_WALL && left_distance < DISTANCE_DESCRIBING_NO_WALL) {
		//The robot has to turn immediately
		turn_left((uint16_t)90);

		//Wait 2 seconds so that the robot finished to turn himself and start a bit to go forward before checking if there is walls
		time2 = chVTGetSystemTime();
		chThdSleepUntilWindowed(time2, time2 + MS2ST(2000));
	} else {
		//The robot has to turn around immediately
		turn_right((uint16_t)180);

		//Wait 2 seconds so that the robot finished to turn himself and start a bit to go forward before checking if there is walls
		time2 = chVTGetSystemTime();
		chThdSleepUntilWindowed(time2, time2 + MS2ST(2000));
	}

}

static THD_WORKING_AREA(waFindPathToExit, 256);
static THD_FUNCTION(FindPathToExit, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    while(1){
        time = chVTGetSystemTime();

        //check_for_path();

        //10Hz
        chThdSleepUntilWindowed(time, time + MS2ST(100));
    }
}

void find_path_to_exit_start(void){
	chThdCreateStatic(waFindPathToExit, sizeof(waFindPathToExit), NORMALPRIO + 1, FindPathToExit, NULL);
}
