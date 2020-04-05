#include "ch.h" //DONT TOUCH : NECESSARY  FOR COMPILATION
#include "hal.h"
#include "memory_protection.h"
//#include <sensors/proximity.h>
#include <chprintf.h> /*needed for*/
#include <usbcfg.h>	 /*the chprintf fonction*/
#include <motors.h>
#include <manage_motors.h>

int main(void)
{
	//OBLIGATOIRE
    halInit();
    chSysInit();
    mpu_init();

    //Init the motors
    motors_init();

    //proximity_start();
	//calibrate_ir();
	/*int distance;
	//chprintf((BaseSequentialStream *)&SDU1,"distance = %d\r\n", distance);*/

    set_direction(right);
    set_direction(left);

	while (1) {

	}
}






//DONT TOUCH : NECESSARY  FOR COMPILATION
#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
