#include <explore.h>
#include <stdio.h>
#include <stdlib.h>
#include "motors.h"
#include "sensors/proximity.h"
#include <math.h>
#include "arm_math.h"
#include "leds.h"



//-----------------------------------------------STATIC VARIABLES-----------------------------------------------------------
static float distance_cm = 5;

static struct position_direction{

	uint8_t current_position [2];
	uint8_t way_ahead_state;
	uint8_t way_right_side_state;
	uint8_t way_left_side_state;
	uint8_t orientation_realtive_direction;				//represents the difference between direction and orientation
														//:1 is turned right, -1 is left, 2 is back
	enum {
		UP=1,
		LEFT=2,
		DOWN=3,
		RIGHT=4
	}desired_direction, current_direction;

	enum {
		CRUISING=1,
		AVOIDING=2
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
    	if (position_direction.action==FORWARD){
    		move_forward(get_goal_distance(), 10);
    	}
    	else{
    		move_turn(90,5);
    	}
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
    		//position_direction.status=CRUISING; doit pas etre la, devrait commencer tho, remis apres evitement
    		position_direction.action=FORWARD;

    		break;
    	case JAMMED:
    		position_direction.status=AVOIDING;
    		position_direction.action=TURNING;
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


void run_away(void){ //retourner en zone "sure"

}


void find_home (void){ //calcul de l'argument mais angle par rapport a l'axe positif des x et donc pas l'angle a appliquer...
	float angle_RTH = 0;
	if (position_direction.current_position[0]>0)
		angle_RTH= atan2 (position_direction.current_position[1],position_direction.current_position[0]) * 180 / PI;
	if ((position_direction.current_position[0]<0) && (position_direction.current_position[1]>0))
		angle_RTH= atan2 (position_direction.current_position[1],position_direction.current_position[0]) * 180 / PI + PI;
	if ((position_direction.current_position[0]<0) && (position_direction.current_position[1]<0))
			angle_RTH= atan2 (position_direction.current_position[1],position_direction.current_position[0]) * 180 / PI - PI;
}


void rotate_right_direction(void){
}

void find_free_direction(void){ //find the right direction without blocus
	if(get_prox(RIGHT_SIDE)>get_prox(LEFT_SIDE)){

	}
}

void go_round_the_inside(void){		////////////////faire ça////////424242424242
	find_free_direction();



//	while(position_direction.status==AVOIDING){
//		is_there_obstacle_right_side();
//		if(position_direction.way_right_side_state==1){
//			move_turn(90,3);
//			move_forward(3,5);
//			continue;
//		}
//		is_there_obstacle_ahead();
//		if(position_direction.way_ahead_state==1){
//			move_forward(3,5);
//			continue;
//		}
//		is_there_obstacle_left_side();
//		if(position_direction.way_left_side_state==1){
//			move_turn(90,-3);
//			move_forward(3,5);
//			continue;
//		}
//		move_turn(180,3);
//	}
}



float get_goal_distance(){
	if ((position_direction.current_direction==UP) || (position_direction.current_direction==DOWN)){
		distance_cm+=5;
	}
	return distance_cm;
}



void change_direction(void){
	++position_direction.current_direction;
	if(position_direction.current_direction ==5){
		position_direction.current_direction=1;
	}
}



void move_turn(float angle, float speed)//je pense c'est OK
{
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	right_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);
	left_motor_set_speed(-speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);


	 while ((abs(left_motor_get_pos()) < abs((angle/FULL_TURN_DEGREES)*STEPS_WHEEL_TURN*CORRECTION_FACTOR))
	    	&& (abs(right_motor_get_pos()) < abs((angle/FULL_TURN_DEGREES)*STEPS_WHEEL_TURN*CORRECTION_FACTOR))) {
		}
	 halt();
	position_direction.action=FORWARD;
	change_direction();
}


void move_forward(float distance, float speed)
{
    //position_direction.status==CRUISING;
	right_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);
	left_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);

	left_motor_set_pos(0);
	right_motor_set_pos(0);

	while ((right_motor_get_pos() < distance* STEPS_WHEEL_TURN / WHEEL_PERIMETER)
														&&(position_direction.status==CRUISING)){

	}
	halt();
	if(position_direction.status==AVOIDING){
		update_coordinate(right_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN);
		go_round_the_inside();
	}
	else{
		position_direction.action=TURNING;
		update_coordinate(distance);
	}

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


void init_position_direction(void){
	position_direction.current_direction=UP;
	position_direction.status=CRUISING;
}
