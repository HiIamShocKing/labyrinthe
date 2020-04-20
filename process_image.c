#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>
#include <leds.h>


#define IMAGE_BUFFER_SIZE		640

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static int counter_red_peach = 0;
static int counter_green_peach = 0;
static int counter_blue_peach = 0;

static int counter_red_fusee = 0;
static int counter_green_fusee = 0;
static int counter_blue_fusee = 0;

static int counter_red_banane = 0;
static int counter_green_banane = 0;
static int counter_blue_banane = 0;

static int counter_red_champignon = 0;
static int counter_green_champignon = 0;
static int counter_blue_champignon = 0;

static THD_WORKING_AREA(waCaptureImage, 1024);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    po8030_set_awb(0);

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
    }
}


static THD_WORKING_AREA(waProcessImage, 2048);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[2*IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_red[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_green[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_blue[IMAGE_BUFFER_SIZE] = {0};
	systime_t time;

    while(1){
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
   		counter_red_peach = 0;
    		counter_green_peach = 0;
    		counter_blue_peach = 0;

    		counter_red_fusee = 0;
    		counter_green_fusee = 0;
    		counter_blue_fusee = 0;

    		counter_red_banane = 0;
    		counter_green_banane = 0;
    		counter_blue_banane = 0;

    		counter_red_champignon = 0;
    		counter_green_champignon = 0;
    		counter_blue_champignon = 0;
		//gets the pointer to the array filled with the last image in RGB565
		img_buff_ptr = dcmi_get_last_image_ptr();

		//Extracts only the red pixels
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			//extracts first 5bits of the first byte
			//takes nothing from the second byte
			image[i] = (uint8_t)img_buff_ptr[i];//&0xF8;
			image[i+1] = (uint8_t)img_buff_ptr[i+1];//&0xF8;
		}

		for(uint16_t i = 0; i < 2*IMAGE_BUFFER_SIZE; i+=2) {

			image_red[i/2] = ((image[i] & 0b11111000) >> 3);
			image_blue[i/2] = (image[i+1] & 0b00011111);
			image_green[i/2] = ((image[i] & 0b00000111) << 3) + ((image[i+1] & 0b11100000) >> 5);

			//Check colors for princesse peach
			if(image_red[i/2] > 25) {
				counter_red_peach ++;
			}
			if(image_green[i/2] > 12 && image_green[i/2] < 20) {
				counter_green_peach ++;
			}
			if(image_blue[i/2] > 16 && image_blue[i/2] < 25) {
				counter_blue_peach ++;
			}

			//Check colors for fusée
			if(image_red[i/2] > 10 && image_red[i/2] < 18) {
				counter_red_fusee ++;
			}
			if(image_green[i/2] > 15 && image_green[i/2] < 30) {
				counter_green_fusee ++;
			}
			if(image_blue[i/2] > 14 && image_blue[i/2] < 21) {
				counter_blue_fusee ++;
			}

			//Check colors for banane
			if(image_red[i/2] > 10 && image_red[i/2] < 18) {
				counter_red_banane ++;
			}
			if(image_green[i/2] > 15 && image_green[i/2] < 37) {
				counter_green_banane ++;
			}
			if(image_blue[i/2] < 5) {
				counter_blue_banane ++;
			}

			//Check colors for champignon
			if(image_red[i/2] > 10 && image_red[i/2] < 17) {
				counter_red_champignon ++;
			}
			if(image_green[i/2] > 17 && image_green[i/2] < 26) {
				counter_green_champignon ++;
			}
			if(image_blue[i/2] > 5 && image_blue[i/2] < 25) {
				counter_blue_champignon ++;
			}

		}
		chprintf((BaseSequentialStream *)&SDU1,"counter_red_peach = %d\r\n",counter_red_peach);
		chprintf((BaseSequentialStream *)&SDU1,"counter_green_peach = %d\r\n",counter_green_peach);
		chprintf((BaseSequentialStream *)&SDU1,"counter_blue_peach = %d\r\n",counter_blue_peach);

		chprintf((BaseSequentialStream *)&SDU1,"counter_red_fusee = %d\r\n",counter_red_fusee);
		chprintf((BaseSequentialStream *)&SDU1,"counter_green_fusee = %d\r\n",counter_green_fusee);
		chprintf((BaseSequentialStream *)&SDU1,"counter_blue_fusee = %d\r\n",counter_blue_fusee);

		chprintf((BaseSequentialStream *)&SDU1,"counter_red_banane = %d\r\n",counter_red_banane);
		chprintf((BaseSequentialStream *)&SDU1,"counter_green_banane = %d\r\n",counter_green_banane);
		chprintf((BaseSequentialStream *)&SDU1,"counter_blue_banane = %d\r\n",counter_blue_banane);

		chprintf((BaseSequentialStream *)&SDU1,"counter_red_champignon = %d\r\n",counter_red_champignon);
		chprintf((BaseSequentialStream *)&SDU1,"counter_green_champignon = %d\r\n",counter_green_champignon);
		chprintf((BaseSequentialStream *)&SDU1,"counter_blue_champignon = %d\r\n",counter_blue_champignon);


		if(counter_red_peach > 100 && counter_green_peach > 100 && counter_blue_peach > 100) {
			//ALLUMER UNE LED
			set_body_led(1);
			time = chVTGetSystemTime();
			chThdSleepUntilWindowed(time, time + MS2ST(2000));
			set_body_led(0);

			DESIRED_SPEED = 0;
		}

		if(counter_red_fusee > 100 && counter_green_fusee > 100 && counter_blue_fusee > 100) {
			//ALLUMER UNE LED
			set_front_led(1);
			time = chVTGetSystemTime();
			chThdSleepUntilWindowed(time, time + MS2ST(2000));
			set_front_led(0);
		}

		if(counter_red_champignon > 100 && counter_green_champignon > 100 && counter_blue_champignon > 100) {
			//ALLUMER UNE LED
			set_led(LED3, 1);
			time = chVTGetSystemTime();
			chThdSleepUntilWindowed(time, time + MS2ST(2000));
			set_led(LED3, 0);
		}


		if(counter_red_banane > 100 && counter_green_banane > 100 && counter_blue_banane > 100) {
			//ALLUMER UNE LED
			set_led(LED1, 1);
			time = chVTGetSystemTime();
			chThdSleepUntilWindowed(time, time + MS2ST(2000));
			set_led(LED1, 0);
		}






    }
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}



/*
#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>


static float distance_cm = 0;
static uint16_t line_position = IMAGE_BUFFER_SIZE/2;	//middle

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);


 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found

uint16_t extract_line_width(uint8_t *buffer){

	uint16_t i = 0, begin = 0, end = 0, width = 0;
	uint8_t stop = 0, wrong_line = 0, line_not_found = 0;
	uint32_t mean = 0;

	static uint16_t last_width = PXTOCM/GOAL_DISTANCE;

	//performs an average
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		mean += buffer[i];
	}
	mean /= IMAGE_BUFFER_SIZE;

	do{
		wrong_line = 0;
		//search for a begin
		while(stop == 0 && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
		{
			//the slope must at least be WIDTH_SLOPE wide and is compared
		    //to the mean of the image
		    if(buffer[i] > mean && buffer[i+WIDTH_SLOPE] < mean)
		    {
		        begin = i;
		        stop = 1;
		    }
		    i++;
		}
		//if a begin was found, search for an end
		if (i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) && begin)
		{
		    stop = 0;

		    while(stop == 0 && i < IMAGE_BUFFER_SIZE)
		    {
		        if(buffer[i] > mean && buffer[i-WIDTH_SLOPE] < mean)
		        {
		            end = i;
		            stop = 1;
		        }
		        i++;
		    }
		    //if an end was not found
		    if (i > IMAGE_BUFFER_SIZE || !end)
		    {
		        line_not_found = 1;
		    }
		}
		else//if no begin was found
		{
		    line_not_found = 1;
		}

		//if a line too small has been detected, continues the search
		if(!line_not_found && (end-begin) < MIN_LINE_WIDTH){
			i = end;
			begin = 0;
			end = 0;
			stop = 0;
			wrong_line = 1;
		}
	}while(wrong_line);

	if(line_not_found){
		begin = 0;
		end = 0;
		width = last_width;
	}else{
		last_width = width = (end - begin);
		line_position = (begin + end)/2; //gives the line position.
	}

	//sets a maximum width or returns the measured width
	if((PXTOCM/width) > MAX_DISTANCE){
		return PXTOCM/MAX_DISTANCE;
	}else{
		return width;
	}
}

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
    }
}


static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};
	uint16_t lineWidth = 0;

	bool send_to_computer = true;

    while(1){
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565
		img_buff_ptr = dcmi_get_last_image_ptr();

		//Extracts only the red pixels
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			//extracts first 5bits of the first byte
			//takes nothing from the second byte
			image[i/2] = (uint8_t)img_buff_ptr[i]&0xF8;
		}

		//search for a line in the image and gets its width in pixels
		lineWidth = extract_line_width(image);

		//converts the width into a distance between the robot and the camera
		if(lineWidth){
			distance_cm = PXTOCM/lineWidth;
		}

		if(send_to_computer){
			//sends to the computer the image
			SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
		}
		//invert the bool
		send_to_computer = !send_to_computer;
    }
}

float get_distance_cm(void){
	return distance_cm;
}

uint16_t get_line_position(void){
	return line_position;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}*/
