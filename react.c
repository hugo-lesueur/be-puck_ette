#include "ch.h"
#include "hal.h"
#include <main.h>
#include <usbcfg.h>
#include <chprintf.h>
#include "leds.h"
#include <motors.h>
#include <audio/microphone.h>
#include <audio_processing.h>
#include <communications.h>
#include <fft.h>
#include <arm_math.h>
#include <react.h>


//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
static float micLeft_cmplx_input[2 * FFT_SIZE];
static float micRight_cmplx_input[2 * FFT_SIZE];
static float micFront_cmplx_input[2 * FFT_SIZE];
static float micBack_cmplx_input[2 * FFT_SIZE];
//Arrays containing the computed magnitude of the complex numbers
static float micLeft_output[FFT_SIZE];
static float micRight_output[FFT_SIZE];
static float micFront_output[FFT_SIZE];
static float micBack_output[FFT_SIZE];

#define MIN_VALUE_THRESHOLD	10000

#define MIN_FREQ_INDEX		24
#define DANGER_FREQ	        26	//406Hz
#define MAX_FREQ_INDEX	    28








static THD_WORKING_AREA(waLookoutDanger, 256);
static THD_FUNCTION(LookoutDanger, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){

    	}

    }



void lookout_danger(float* data){
	float max_norm = MIN_VALUE_THRESHOLD;
	int16_t max_norm_index = -1;

	//search for the highest peak
	for(uint16_t i = MIN_FREQ_INDEX ; i <= MAX_FREQ_INDEX ; i++){
		if(data[i] > max_norm){
			max_norm = data[i];
			max_norm_index = i;
		}
	}

	if(max_norm_index >= MIN_FREQ_INDEX && max_norm_index <= MAX_FREQ_INDEX){
		set_led(LED7,100);
		set_led(LED1,100);

		//appel à la fonction de retour
	}
	else{
		clear_leds();
	}
}

	void processAudioData(int16_t *data, uint16_t num_samples){

		/*
		*
		*	We get 160 samples per mic every 10ms
		*	So we fill the samples buffers to reach
		*	1024 samples, then we compute the FFTs.
		*
		*/

		static uint16_t nb_samples = 0;
		static uint8_t mustSend = 0;

		//loop to fill the buffers
		for(uint16_t i = 0 ; i < num_samples ; i+=4){
			//construct an array of complex numbers. Put 0 to the imaginary part
			micRight_cmplx_input[nb_samples] = (float)data[i + MIC_RIGHT];


			nb_samples++;

			micRight_cmplx_input[nb_samples] = 0;
			nb_samples++;

			//stop when buffer is full
			if(nb_samples >= (2 * FFT_SIZE)){
				break;
			}
		}

		if(nb_samples >= (2 * FFT_SIZE)){


			doFFT_optimized(FFT_SIZE, micRight_cmplx_input);
			arm_cmplx_mag_f32(micRight_cmplx_input, micRight_output, FFT_SIZE);
			lookout_danger(micLeft_output);
		}
	}



	float* get_audio_buffer_ptr(BUFFER_NAME_t name){
		if(name == LEFT_CMPLX_INPUT){
			return micLeft_cmplx_input;
		}
		else if (name == RIGHT_CMPLX_INPUT){
			return micRight_cmplx_input;
		}
		else if (name == FRONT_CMPLX_INPUT){
			return micFront_cmplx_input;
		}
		else if (name == BACK_CMPLX_INPUT){
			return micBack_cmplx_input;
		}
		else if (name == LEFT_OUTPUT){
			return micLeft_output;
		}
		else if (name == RIGHT_OUTPUT){
			return micRight_output;
		}
		else if (name == FRONT_OUTPUT){
			return micFront_output;
		}
		else if (name == BACK_OUTPUT){
			return micBack_output;
		}
		else{
			return NULL;
		}
	}



void LookoutDanger_start(void){
	chThdCreateStatic(waLookoutDanger, sizeof(waLookoutDanger), NORMALPRIO, LookoutDanger, NULL);
}
