#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <motors.h>
#include <audio/microphone.h>
#include <react.h>
#include <fft.h>
#include <arm_math.h>
#include "explore.h"
#include "react.h"
#include <leds.h>
#include "sdio.h"
#include "sensors/proximity.h"
#include "spi_comm.h"
#include "usbcfg.h"
#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"

#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define MIN_LINE_WIDTH			40
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			2
#define PXTOCM					1570.0f //experimental value
#define ERROR_THRESHOLD			0.1f	//[cm] because of the noise of the camera
#define MAX_DISTANCE 			25.0f
#define GOAL_DISTANCE 			10.0f
#define KP						800.0f
#define KI 						3.5f	//must not be zero
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/KI)

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

#ifdef __cplusplus
}
#endif

#endif
