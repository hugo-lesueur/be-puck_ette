#ifndef EXPLORE_H
#define EXPLORE_H


#include <stdio.h>
#include <stdlib.h>
#include <explore.h>
#include "motors.h"
#include "sensors/proximity.h"
#include "leds.h"




#define FALSE             0
#define TRUE              1
#define JAMMED		      0
#define FREE              1
#define FRONT_LEFT        7
#define FRONT_RIGHT       0
#define RIGHT_SIDE        2
#define LEFT_SIDE         5
#define OBSTACLE_DISTANCE 300
#define CORRECTION_FACTOR 1.291    // correct the angle of rotation to be more precise
#define CORRECTION_FORWARD /*0.95*/ 1 //correct the distance
#define SPEED 8
#define WHEEL_AXIS_WIDTH  5.35f                  //[cm]
#define PERIMETER_EPUCK   (PI * WHEEL_AXIS_WIDTH)
#define WHEEL_PERIMETER   13                     // [cm]
#define FULL_TURN_DEGREES 360.0f                 // degrees for a whole revolution
#define STEPS_WHEEL_TURN  1000					 //number of steps per wheel revolution
#define EPUCK_RADIUS      4     				 //cm
#define ERROR_POS_RTH 	5



//---------------------------------Initialisation Threads---------------------------------------------
void Move_start(void);
void ObstacleInspector_start(void);
void init_position_direction(void);

//---------------------------------Detection---------------------------------------------
void is_there_obstacle_ahead(void);

//---------------------------------Movement---------------------------------------------
void rotate_right_direction_y(void);
void rotate_right_direction_x(void);
void update_direction(void);
void move_turn(float position, float speed);
void motor_reboot(void);
void move(float speed);
float get_goal_distance(void);
void change_direction(void);
void halt (void);
void move_forward(float distance, float speed);
void update_coordinate (int32_t distance);


//---------------------------------avoid obstacle---------------------------------------------
void avoid_obstacle(void);

//---------------------------------Return to Home---------------------------------------------
void RTH(void);
void set_to_flee (void);

#endif /* EXPLORE_H*/
