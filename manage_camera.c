#include "manage_camera.h"

#include "ch.h"
#include "hal.h"

#include <main.h>
#include <camera/po8030.h>
#include <manage_motors.h>
#include <leds.h>
#include <stdbool.h>
#include <sensors/proximity.h>
#include <find_path_to_exit.h>


#define IMAGE_BUFFER_SIZE	640

#define MEAN_RED_MIN_PINK	4
#define MEAN_RED_MAX_PINK 	6
#define MEAN_GREEN_MIN_PINK 	5
#define MEAN_GREEN_MAX_PINK 	7
#define MEAN_BLUE_MIN_PINK	3
#define MEAN_BLUE_MAX_PINK	5

#define MEAN_RED_MIN_WHITE	5
#define MEAN_RED_MAX_WHITE	7
#define MEAN_GREEN_MIN_WHITE 17
#define MEAN_GREEN_MAX_WHITE 19
#define MEAN_BLUE_MIN_WHITE	9
#define MEAN_BLUE_MAX_WHITE	11

#define MEAN_RED_MIN_BLUE	1
#define MEAN_RED_MAX_BLUE	3
#define MEAN_GREEN_MIN_BLUE	9
#define MEAN_GREEN_MAX_BLUE	12
#define MEAN_BLUE_MIN_BLUE	7
#define MEAN_BLUE_MAX_BLUE	9

#define MEAN_RED_MIN_GREEN	2
#define MEAN_RED_MAX_GREEN	5
#define MEAN_GREEN_MIN_GREEN	14
#define MEAN_GREEN_MAX_GREEN	17
#define MEAN_BLUE_MIN_GREEN	5
#define MEAN_BLUE_MAX_GREEN	7

#define FAST_SPEED_COEFF			0.4
#define SUPER_FAST_SPEED_COEFF	0.6
#define SLOW_SPEED_COEFF			0.2

#define DANSE_NUMBER	2

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
	set_desired_speed((int)(SUPER_FAST_SPEED_COEFF * (float)MOTOR_SPEED_LIMIT));

	if(counter_dance == 0){
		turn_left(QUARTER_TURN);
		turn_right(HALF_TURN);
	} else if (counter_dance == DANSE_NUMBER) {
		turn_left(HALF_TURN);
		turn_right(QUARTER_TURN);
	} else {
		turn_left(HALF_TURN);
		turn_right(HALF_TURN);
	}

	counter_dance++;
}

/**
* @brief   Analyse the image and manage 3 arrays of colors (RGB),
* 		   then do an average of each one : mean_red, mean_green, mean_blue (static variables)
*
*/
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

/**
* @brief   change robot's behavior by looking at the mean_red, mean_green, mean_blue (static variables)
*
*/
void manage_robot(void){
	if(get_see_red() == true){
		if(counter_dance <= DANSE_NUMBER){
			dance();
		} else {
			set_desired_speed(0);
		}
	}
	else if(get_front_wall() == true){
		if((mean_red >= MEAN_RED_MIN_PINK && mean_red <= MEAN_RED_MAX_PINK &&
			mean_green >= MEAN_GREEN_MIN_PINK && mean_green <= MEAN_GREEN_MAX_PINK &&
			mean_blue >= MEAN_BLUE_MIN_PINK && mean_blue <= MEAN_BLUE_MAX_PINK)) {
			set_see_red(true);
			set_led_color(pink);
			playMelody(WE_ARE_THE_CHAMPIONS, ML_FORCE_CHANGE, NULL);
		} else if(mean_red >= MEAN_RED_MIN_WHITE && mean_red <= MEAN_RED_MAX_WHITE &&
				mean_green >= MEAN_GREEN_MIN_WHITE && mean_green <= MEAN_GREEN_MAX_WHITE &&
				mean_blue >= MEAN_BLUE_MIN_WHITE && mean_blue <= MEAN_BLUE_MAX_WHITE) {
			playMelody(MARIO_DEATH, ML_FORCE_CHANGE, NULL);
			set_see_white(true);
			set_led_color(white);
		} else if(mean_red >= MEAN_RED_MIN_BLUE && mean_red <= MEAN_RED_MAX_BLUE &&
				mean_green >= MEAN_GREEN_MIN_BLUE && mean_green <= MEAN_GREEN_MAX_BLUE &&
				mean_blue >= MEAN_BLUE_MIN_BLUE && mean_blue <= MEAN_BLUE_MAX_BLUE){
			playMelody(MARIO_FLAG, ML_FORCE_CHANGE, NULL);
			set_toggle_period_rgb_led(fast);
			set_led_color(blue);
			set_desired_speed((int)(FAST_SPEED_COEFF * (float)MOTOR_SPEED_LIMIT));
		} else if (mean_red >= MEAN_RED_MIN_GREEN && mean_red <= MEAN_RED_MAX_GREEN &&
				mean_green >= MEAN_GREEN_MIN_GREEN && mean_green <= MEAN_GREEN_MAX_GREEN &&
				mean_blue >= MEAN_BLUE_MIN_GREEN && mean_blue <= MEAN_BLUE_MAX_GREEN){
			playMelody(MARIO_FLAG, ML_FORCE_CHANGE, NULL);
			set_toggle_period_rgb_led(slow);
			set_led_color(green);
			set_desired_speed((int)(SLOW_SPEED_COEFF * (float)MOTOR_SPEED_LIMIT));
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

	//wait the system to be ready to disable the auto white-balance
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
    }
}

void manage_camera_start(void){
	chThdCreateStatic(waManageCamera, sizeof(waManageCamera), NORMALPRIO, ManageCamera, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
