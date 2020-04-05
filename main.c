#include "ch.h" //DONT TOUCH : NECESSARY  FOR COMPILATION
#include "hal.h"
#include "memory_protection.h"
//#include <sensors/proximity.h>
//#include <chprintf.h>
//#include <usbcfg.h>
#include <leds.h>
#include <motors.h>

static int32_t pos = -1;
enum direction {
	forward, backward, left, right
};


void set_direction_robot(enum direction direction) {
	switch(direction) {
	case forward:
		left_motor_set_speed(1100);
	    right_motor_set_speed(1100);
		for(int i=0; i< 84000000; i++)
		{
			__asm__("nop");
		}
		left_motor_set_speed(0);
		right_motor_set_speed(0);
		break;
	case backward:
		left_motor_set_speed(-1100);
	    right_motor_set_speed(-1100);
		break;
	case left:
		left_motor_set_pos((int32_t)10);
	    right_motor_set_pos((int32_t)10);
		//left_motor_set_speed(-1100);
	    //right_motor_set_speed(1100);
		break;
	case right:
		left_motor_set_speed(1100);
	    right_motor_set_speed(-1100);
		break;
	}
}

int main(void)
{
	//OBLIGATOIRE
    halInit();
    chSysInit();
    mpu_init();

    //Init the motors
    motors_init();

    /*proximity_start();
	calibrate_ir();
	int distance;
	//chprintf((BaseSequentialStream *)&SDU1,"distance = %d\r\n", distance);*/



	//left_motor_set_speed(1100);
	//right_motor_set_speed(1100);
    set_direction_robot(left);

   // pos = left_motor_get_pos();

	while (1) {
	  //  pos = left_motor_get_pos();
	}
}






//DONT TOUCH : NECESSARY  FOR COMPILATION
#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
