#include "ch.h" //DONT TOUCH : NECESSARY  FOR COMPILATION
#include "hal.h"
#include "memory_protection.h"

#include <main.h>
#include <sensors/proximity.h>
#include <chprintf.h> //needed for
#include <usbcfg.h>	 //the chprintf fonction
#include <motors.h>
#include <manage_motors.h>
#include <find_path_to_exit.h>
#include <process_image.h>

messagebus_t bus;             //needed to
MUTEX_DECL(bus_lock);         //be able to use
CONDVAR_DECL(bus_condvar);    //proximity sensors


int main(void)
{

	//OBLIGATOIRE
    halInit();
    chSysInit();
    mpu_init();

    //start the USB communication
    usb_start();

    //starts the camera
    dcmi_start();
	po8030_start();

	//init the motors
	motors_init();

	//Init message bus, needed to use the proximity sensors
	messagebus_init(&bus, &bus_lock, &bus_condvar);

	//start and calibrate proximity sensors
    proximity_start();
	calibrate_ir();

	//stars the thread to find the exit of the labyrinthe
	find_path_to_exit_start();
	process_image_start();

	while (1) {
		chThdSleepMilliseconds(1000); //wait 1s
	}
}






//DONT TOUCH : NECESSARY  FOR COMPILATION
#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
