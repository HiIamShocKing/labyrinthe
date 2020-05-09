#include "ch.h" 					//DONT TOUCH : NECESSARY  FOR COMPILATION
#include "hal.h"
#include "memory_protection.h"

#include <main.h>
#include <sensors/proximity.h>
#include <motors.h>
#include <leds.h>
#include <find_path_to_exit.h>
#include <spi_comm.h>


#include <chprintf.h> //needed for
#include <usbcfg.h>	 //the chprintf fonction
#include "manage_camera.h"

//needed to be able to use proximity sensors
messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

static toggle_speed toggle_period_rgb_led = normal;
static rgb_led_color led_color = yellow;
static bool leds_on = true;

bool get_leds_statue(void){
	return leds_on;
}

void set_leds_statue(bool boolean){
	leds_on = boolean;
}

toggle_speed get_toggle_period_rgb_led(void){
	return toggle_period_rgb_led;
}

void set_toggle_period_rgb_led(toggle_speed state){
	toggle_period_rgb_led = state;
}

rgb_led_color get_led_color(void){
	return led_color;
}

void set_led_color(rgb_led_color color){
	led_color = color;
}


void turn_on_led(void){
	switch(get_led_color()){
	case green:
		set_rgb_led(LED2, 0, 255, 0);
		set_rgb_led(LED4, 0, 255, 0);
		set_rgb_led(LED6, 0, 255, 0);
		set_rgb_led(LED8, 0, 255, 0);
		break;
	case white:
		set_rgb_led(LED2, 255, 255, 255);
		set_rgb_led(LED4, 255, 255, 255);
		set_rgb_led(LED6, 255, 255, 255);
		set_rgb_led(LED8, 255, 255, 255);
		break;
	case blue:
		set_rgb_led(LED2, 0, 0, 255);
		set_rgb_led(LED4, 0, 0, 255);
		set_rgb_led(LED6, 0, 0, 255);
		set_rgb_led(LED8, 0, 0, 255);
		break;
	case yellow:
		set_rgb_led(LED2, 255, 255, 0);
		set_rgb_led(LED4, 255, 255, 0);
		set_rgb_led(LED6, 255, 255, 0);
		set_rgb_led(LED8, 255, 255, 0);
		break;
	case pink:
		set_rgb_led(LED2, 255, 0, 255);
		set_rgb_led(LED4, 255, 0, 255);
		set_rgb_led(LED6, 255, 0, 255);
		set_rgb_led(LED8, 255, 0, 255);
		break;
	}
}

void turn_off_led(void){
	set_rgb_led(LED2, 0, 0, 0);
	set_rgb_led(LED4, 0, 0, 0);
	set_rgb_led(LED6, 0, 0, 0);
	set_rgb_led(LED8, 0, 0, 0);
}

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

void toggle_led(void){
	turn_on_led();
	chThdSleepMilliseconds(get_toggle_period_rgb_led());
	turn_off_led();
	chThdSleepMilliseconds(get_toggle_period_rgb_led());
}


int main(void)
{
	//OBLIGATION
    halInit();
    chSysInit();
    mpu_init();

    serial_start();

    //start the USB communication
    usb_start();

	//start thread for RGB leds
	spi_comm_start();

	//Starts the DAC module. Power of the audio amplifier and DAC peripheral
	dac_start();

	//starts the thread for melody
	playMelodyStart();

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

	//playMelody(MARIO_START, ML_FORCE_CHANGE, NULL);

	//starts the thread to find the exit of the labyrinth
	find_path_to_exit_start();

	//starts the thread to act while using the camera
	manage_camera_start();

	while (1) {
		if(get_leds_statue() == true){
			toggle_led();
			chThdSleepMilliseconds(get_toggle_period_rgb_led());
		} else {
			turn_off_led();
			chThdSleepMilliseconds(get_toggle_period_rgb_led());
		}
	}
}


//DONT TOUCH : NECESSARY  FOR COMPILATION
#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
