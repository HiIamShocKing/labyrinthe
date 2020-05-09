#include "manage_camera.h"

#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>
#include <manage_motors.h>
#include <leds.h>
#include <stdbool.h>
#include <sensors/proximity.h>
#include <find_path_to_exit.h>

#define IMAGE_BUFFER_SIZE		640

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static uint8_t *img_buff_ptr;
static uint8_t image[2*IMAGE_BUFFER_SIZE] = {0};
static uint8_t image_red[IMAGE_BUFFER_SIZE] = {0};
static uint8_t image_green[IMAGE_BUFFER_SIZE] = {0};
static uint8_t image_blue[IMAGE_BUFFER_SIZE] = {0};
static uint16_t mean_red = 0;
static uint16_t mean_green = 0;
static uint16_t mean_blue = 0;
static uint8_t counter_dance = 0;
static bool see_white = false;
static bool see_red = false;

bool get_see_white(void){
	return see_white;
}

void set_see_white(bool boolean){
	see_white = boolean;
}

bool get_see_red(void){
	return see_red;
}

void set_see_red(bool boolean){
	see_red = boolean;
}

void dance(void){
	set_desired_speed((int)(0.6 * (float)MOTOR_SPEED_LIMIT));

	if(counter_dance == 0){
		turn_left(90);
		turn_right(180);
	} else if (counter_dance == 2) {
		turn_left(180);
		turn_right(90);
	} else {
		turn_left(180);
		turn_right(180);
	}

	counter_dance++;
}

void process_image(void){
	//waits until an image has been captured
    chBSemWait(&image_ready_sem);

    //Reset
	mean_red = 0;
	mean_green = 0;
	mean_blue = 0;

	//gets the pointer to the array filled with the last image in RGB565
	img_buff_ptr = dcmi_get_last_image_ptr();

	//Extracts RGB pixels
	for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
		//extract color bits
		image[i] = (uint8_t)img_buff_ptr[i]; //Represent the register with RRRRRGGG
		image[i+1] = (uint8_t)img_buff_ptr[i+1]; //Represent the register with GGGBBBBB

		//The two registers are composed like this: 	//(MSB)RRRRR(LSB)(MSB)GGG
													//GGG(LSB)(MSB)BBBBB(LSB)
		//Filter of RGB colors
		image_red[i/2] = ((image[i] & 0b11111000) >> 3);
		image_blue[i/2] = (image[i+1] & 0b00011111);
		image_green[i/2] = ((image[i] & 0b00000111) << 3) + ((image[i+1] & 0b11100000) >> 5);

		mean_red += image_red[i/2];
		mean_green += image_green[i/2];
		mean_blue += image_blue[i/2];
	}

	//Computes the mean value of Red, Green and Blue seen by the camera
	mean_red /= IMAGE_BUFFER_SIZE;
	mean_green /= IMAGE_BUFFER_SIZE;
	mean_blue /= IMAGE_BUFFER_SIZE;
}

void manage_robot(void){
	if(get_see_red() == true){
		if(counter_dance <= 2){
			dance();
		} else {
			set_desired_speed(0);
		}
	}
	else if(get_front_wall() == true){
		if((mean_red >= 4 && mean_red <= 6 &&
			mean_green >= 5 && mean_green <= 7 &&
			mean_blue >= 3 && mean_blue <= 5)) {
			set_see_red(true);
			set_led_color(pink);
			/*set_led(LED1, 1);
			set_led(LED3, 0);
			set_led(LED5, 0);
			set_led(LED7, 0);*/
			//playMelody(WE_ARE_THE_CHAMPIONS, ML_FORCE_CHANGE, NULL);
		} else if(mean_red >= 5 && mean_red <= 7 &&
				mean_green >= 17 && mean_green <= 19 &&
				mean_blue >= 9 && mean_blue <= 11) {
			//playMelody(MARIO_DEATH, ML_FORCE_CHANGE, NULL);
			//if(get_prox(FRONT_RIGHT17) > 150 && get_is_turning() == false && see_white == false){
			set_see_white(true);
			set_led_color(white);
				/*set_led(LED1, 0);
				set_led(LED3, 0);
				set_led(LED5, 1);
				set_led(LED7, 0);*/
			//}
		} else if(mean_red >= 1 && mean_red <= 3 &&
				mean_green >= 9 && mean_green <= 12 &&
				mean_blue >= 8 && mean_blue <= 10){
			//playMelody(MARIO_FLAG, ML_FORCE_CHANGE, NULL);
			set_toggle_period_rgb_led(fast);
			set_led_color(blue);
			/*set_led(LED1, 0);
			set_led(LED3, 0);
			set_led(LED5, 0);
			set_led(LED7, 1);*/
			set_desired_speed((int)(0.4 * (float)MOTOR_SPEED_LIMIT));
		}else if (mean_red >= 3 && mean_red <= 6 &&
				mean_green >= 18 && mean_green <= 21 &&
				mean_blue >= 7 && mean_blue <= 11){//6 20 8
			//playMelody(MARIO_FLAG, ML_FORCE_CHANGE, NULL);
			set_toggle_period_rgb_led(slow);
			set_led_color(green);
			/*set_led(LED1, 0);
			set_led(LED3, 1);
			set_led(LED5, 0);
			set_led(LED7, 0);*/
			set_desired_speed((int)(0.2 * (float)MOTOR_SPEED_LIMIT));
		}
	}
}


static THD_WORKING_AREA(waCaptureImage, 1024);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

	//wait the system to be ok to disable the auto white-balance
	chThdSleepMilliseconds(10);
	po8030_set_awb(0);

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
    }
}

static THD_WORKING_AREA(waManageCamera, 1024);
static THD_FUNCTION(ManageCamera, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
    		process_image();
    		manage_robot();

		chprintf((BaseSequentialStream *)&SD3,"mean_red = %d\r\n",mean_red);
		chprintf((BaseSequentialStream *)&SD3,"mean_green = %d\r\n",mean_green);
		chprintf((BaseSequentialStream *)&SD3,"mean_blue = %d\r\n",mean_blue);

    }
}

void manage_camera_start(void){
	chThdCreateStatic(waManageCamera, sizeof(waManageCamera), NORMALPRIO, ManageCamera, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
