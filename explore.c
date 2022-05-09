#include <explore.h>
#include <stdio.h>
#include <stdlib.h>
#include "motors.h"
#include "sensors/proximity.h"
#include <math.h>
#include "arm_math.h"
#include "leds.h"



//-------------------------SEMAPHORE---------------------------------
static BSEMAPHORE_DECL(no_obstacle_sem, TRUE);
static BSEMAPHORE_DECL(goal_not_reached_sem, FALSE); //FALSE so MovementControl can start first

//-----------------------------------------------STATIC VARIABLES-----------------------------------------------------------
static float distance_cm = 5;

static struct position_direction{

	uint8_t current_position [2];
	uint8_t way_ahead_state;
	uint8_t way_right_side_state;
	uint8_t way_left_side_state;
	uint8_t orientation_realtive_direction;				//represents the difference between direction and orientation
														//:1 is turned right, -1 is left, 2 is back
	uint32_t eloignement; //distance de l'eloignement par rapport a la traj prevue

	enum {
		UP=1,
		LEFT=2,
		DOWN=3,
		RIGHT=4
	}desired_direction, current_direction,futur_direction;

	enum {
		ON=1,
		OFF=0
	}track;

	enum {
		CRUISING=1,
		AVOIDING=2,
		FUITE=3
	}status;

	enum{
		FORWARD=1,
		TURNING=2
	}action;

}position_direction;
//-----------------------------------------------THREADS--------------------------------------------------------------------


static THD_WORKING_AREA(waMove, 256);
static THD_FUNCTION(Move, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
    	led_update();
    	if (position_direction.status==CRUISING){
			if (position_direction.action==FORWARD){
				move_forward(get_goal_distance(), 10);
				position_direction.action=TURNING;
			}
			else{
				move_turn(90,5);
				position_direction.action=CRUISING;
			}
    	}
    	if (position_direction.status==AVOIDING) go_round_the_inside();
    	if (position_direction.status==FUITE) RTH();
    }
}

void Move_start(void){
	chThdCreateStatic(waMove, sizeof(waMove), NORMALPRIO, Move, NULL);
}



static THD_WORKING_AREA(waObstacleInspector, 256);
static THD_FUNCTION(ObstacleInspector, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
    	is_there_obstacle_ahead();

    	switch(position_direction.way_ahead_state){
    	case FREE:
    		position_direction.status=CRUISING; //doit pas etre la, devrait commencer tho, remis apres evitement
    		//position_direction.action=FORWARD;

    		break;
    	case JAMMED:
    		position_direction.status=AVOIDING;
    		//position_direction.action=TURNING;
    		break;
    	}

    }
}

void ObstacleInspector_start(void){
	chThdCreateStatic(waObstacleInspector, sizeof(waObstacleInspector), NORMALPRIO, ObstacleInspector, NULL);
}

//-----------------------------------------------INTERNAL FUNCTIONS---------------------------------------------------------

void is_there_obstacle_ahead(void){
	//lit les distances et dit si devant on a quelque chose à 2cm, et change la valeur de position_direction.way_ahead_state
	 if ((get_prox(FRONT_LEFT) > OBSTACLE_DISTANCE) ||
		    (get_prox(FRONT_RIGHT) > OBSTACLE_DISTANCE)) {
		 position_direction.way_ahead_state=JAMMED;
	    }
		else {
			position_direction.way_ahead_state=FREE;
		}
	}


void is_there_obstacle_right_side(void){
	//lit les distances et dit si devant on a quelque chose à 2cm, et change la valeur de position_direction.way_ahead_state
	 if (get_prox(RIGHT_SIDE) > OBSTACLE_DISTANCE) {
		 position_direction.way_right_side_state=JAMMED;
	    }
		else {
			position_direction.way_right_side_state=FREE;
		}
}


void is_there_obstacle_left_side(void){
	//lit les distances et dit si devant on a quelque chose à 2cm, et change la valeur de position_direction.way_ahead_state
	 if (get_prox(LEFT_SIDE) > OBSTACLE_DISTANCE) {
		 position_direction.way_left_side_state=JAMMED;
	    }
		else {
			position_direction.way_left_side_state=FREE;
		}
	}


void RTH(void){ //retourner en zone "sure"
	rotate_right_direction_y();
	move_forward(position_direction.current_position[1],9);
	rotate_right_direction_x();
	move_forward(position_direction.current_position[0],9);
	chThdSleepSeconds(2); // repos du guerrier
	position_direction.status=CRUISING;
	return;
}



void rotate_right_direction_y(void){
	if (position_direction.current_position[0]>0){
		if(position_direction.current_direction==UP)move_turn(180,6);
		if(position_direction.current_direction==LEFT)move_turn(90,6);
		if(position_direction.current_direction==RIGHT)move_turn(90,-6);
		position_direction.current_direction=DOWN;
		return;
	}
	if (position_direction.current_position[0]<=0){
		if(position_direction.current_direction==DOWN)move_turn(180,6);
		if(position_direction.current_direction==LEFT)move_turn(90,-6);
		if(position_direction.current_direction==RIGHT)move_turn(90,6);
		position_direction.current_direction=UP;
		return;
	}
}


void rotate_right_direction_x(void){
	if (position_direction.current_position[1]>0){
		if(position_direction.current_direction==UP)move_turn(90,6);
		else move_turn(90,-6);
		position_direction.current_direction=LEFT;
		return;
	}
	else{
		if(position_direction.current_direction==UP)move_turn(90,-6);
		else move_turn(90,6);
		position_direction.current_direction=RIGHT;
		return;
	}
}


void go_round_the_inside(void){		//avoid obstacle
	position_direction.status=AVOIDING;   //not on track

	is_there_obstacle_left_side();
	if(position_direction.way_left_side_state==1){
		position_direction.desired_direction=LEFT;
		avoid_obstacle();
//		move_forward(3,5);
		return;
	}//utile?
	is_there_obstacle_right_side();
	if(position_direction.way_right_side_state==1){
		position_direction.desired_direction=RIGHT;
		avoid_obstacle();
//		move_forward(3,5);
		return;
	}//pour quoi?
	is_there_obstacle_ahead();
	if(position_direction.way_ahead_state==1){
		move_forward(3,5);
		return;
	}
}

void avoid_obstacle(void){
//------------------------right side-------------------------
	if(position_direction.desired_direction==RIGHT){
		motor_reboot();
		move_turn(90,-6);
		position_direction.futur_direction=RIGHT;
		change_direction();
		while(get_prox(LEFT_SIDE)>OBSTACLE_DISTANCE){
			position_direction.eloignement= left_motor_get_pos(); //retiens distance d'eloignement
			move(6);
		}
		move_forward(EPUCK_RADIUS,6); //advance to not hit the wall
		move_turn(90,6);
		position_direction.futur_direction=LEFT;
		change_direction();
		move_forward(EPUCK_RADIUS,6); //advance to detect smth
//-----------------------side--------------------------------------
		while(get_prox(LEFT_SIDE)>OBSTACLE_DISTANCE){
			move(6);
		}
		move_forward(EPUCK_RADIUS,6); //advance to not hit the wall
		move_turn(90,6);
		move_forward(EPUCK_RADIUS,6); //advance to detect smth
		position_direction.futur_direction=LEFT;
		change_direction();

//----------------comeback on right track--------------------------
		set_led(LED7,100);
		move_forward(position_direction.eloignement* STEPS_WHEEL_TURN / WHEEL_PERIMETER,6);
		move_turn(90,-6);
		position_direction.futur_direction=RIGHT;
		change_direction();
		position_direction.status=CRUISING;

	}



//--------------------------------LEFT SIDE-------------------------------------------
	if(position_direction.desired_direction==LEFT){
		move_turn(90,6);
		motor_reboot();
		position_direction.futur_direction=LEFT;
		change_direction();

//-------------------------first slide------------------------------
		while(get_prox(RIGHT_SIDE)>OBSTACLE_DISTANCE){
			move(6);
		}
		position_direction.eloignement= left_motor_get_pos(); //retient distance d'eloignement
		if(position_direction.eloignement==0){
			set_led(LED1,100);
		}
		change_direction();
		move_forward(EPUCK_RADIUS,6); //advance to not hit the wall
		position_direction.futur_direction=RIGHT;
		change_direction();
		move_turn(90,-6);
		move_forward(EPUCK_RADIUS*2,6); //advance to detect smth
//-----------------------side--------------------------------------
		while(get_prox(RIGHT_SIDE)>OBSTACLE_DISTANCE/2){
			move(6);
		}
		move_forward(EPUCK_RADIUS,6); //advance to not hit the wall
		move_turn(90,-6);
		position_direction.futur_direction=RIGHT;
		change_direction();
		move_forward(EPUCK_RADIUS,6); //advance to detect smth
//----------------comeback on right track--------------------------
		set_led(LED7,100);
		move_forward(position_direction.eloignement*WHEEL_PERIMETER*CORRECTION_FORWARD/STEPS_WHEEL_TURN ,6);
		move_turn(90,6);
		position_direction.futur_direction=LEFT;
		change_direction();
		position_direction.status=CRUISING;
		position_direction.eloignement=0;
		clear_leds();
	}
}

float get_goal_distance(){
	if ((position_direction.current_direction==UP) || (position_direction.current_direction==DOWN)){
		distance_cm+=5;
	}
	return distance_cm;
}



void change_direction(void){
	if (position_direction.futur_direction==LEFT){
		++position_direction.current_direction;
		if(position_direction.current_direction ==5){
			position_direction.current_direction=1;
		}
	}
	if (position_direction.futur_direction==RIGHT){
		--position_direction.current_direction;
		if(position_direction.current_direction ==0){
			position_direction.current_direction=4;
		}
	}
}



void move_turn(float angle, float speed)// speed > 0 --> left,  speed <0 --> right
{
	motor_reboot();

	right_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);
	left_motor_set_speed(-speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);

	while ((abs(left_motor_get_pos()) < abs((angle/FULL_TURN_DEGREES)*STEPS_WHEEL_TURN*CORRECTION_FACTOR))
	    	&& (abs(right_motor_get_pos()) < abs((angle/FULL_TURN_DEGREES)*STEPS_WHEEL_TURN*CORRECTION_FACTOR))) {
		}
	halt();
	position_direction.action=FORWARD;
	change_direction();
}

void move(float speed){
	right_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);
	left_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);
}

void motor_reboot(void){ //pos=0
	left_motor_set_pos(0);
	right_motor_set_pos(0);
}

void move_forward(float distance, float speed)
{
	motor_reboot();

	move(speed);

	while ((right_motor_get_pos() < distance* STEPS_WHEEL_TURN / WHEEL_PERIMETER) && (position_direction.status==CRUISING)){

	}
	halt();
//	if(position_direction.status==AVOIDING){
//		update_coordinate(right_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN);
//		go_round_the_inside();
//	}
//	else{
		update_coordinate(distance);
//	}

}

void halt (void){
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}

void update_coordinate (float distance){
	//coordoninate update
	if (position_direction.current_direction==UP)
		position_direction.current_position[1] +=distance;
	if (position_direction.current_direction==DOWN)
			position_direction.current_position[1] -=distance;
	if (position_direction.current_direction==RIGHT)
			position_direction.current_position[0] +=distance;
	if (position_direction.current_direction==LEFT)
			position_direction.current_position[0] -=distance;
}

void led_update(void){
	if (position_direction.status==CRUISING) {
		set_rgb_led(LED2,0,100,0);
		set_rgb_led(LED3,0,100,0);
		set_rgb_led(LED4,0,100,0);
		set_rgb_led(LED5,0,100,0);
	}
	if (position_direction.status==AVOIDING) {
		set_rgb_led(LED2,100,50,0);
		set_rgb_led(LED3,100,50,0);
		set_rgb_led(LED4,100,50,0);
		set_rgb_led(LED5,100,50,0);
	}
	if (position_direction.status==FUITE) {
		set_rgb_led(LED2,100,0,0);
		set_rgb_led(LED3,100,0,0);
		set_rgb_led(LED4,100,0,0);
		set_rgb_led(LED5,100,0,0);
	}
}

void init_position_direction(void){
	position_direction.current_direction=UP;
	position_direction.status=CRUISING;
	position_direction.action=FORWARD;
	return;
}
