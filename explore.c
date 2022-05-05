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

//	uint8_t position[2] = {0, 0};

//asdfsadfsadf

	uint8_t current_position [2];
	uint8_t way_ahead_state;
	uint8_t way_right_side_state;
	uint8_t way_left_side_state;
	uint8_t orientation_realtive_direction;				//represents the difference between direction and orientation
														//:1 is turned right, -1 is left, 2 is back
	uint8_t x_prior_avoiding;
	uint8_t y_prior_avoiding;
	uint8_t direction_prior_avoiding;
	enum {
		UP=1,
		DOWN=3,
		RIGHT=4,
		LEFT=2

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
    	if (position_direction.action==FORWARD)
    		move_forward(get_goal_distance(), 5);
    	else
    		move_turn(90,3);
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
	//lit les distances et dit si devant on a quelque chose � 2cm, et change la valeur de position_direction.way_ahead_state
	 if ((get_prox(FRONT_LEFT) > OBSTACLE_DISTANCE) ||
		    (get_prox(FRONT_RIGHT) > OBSTACLE_DISTANCE)) {
		 position_direction.way_ahead_state=JAMMED;

	    }
		else {
			position_direction.way_ahead_state=FREE;
		}
	}
void is_there_obstacle_right_side(void){
	//lit les distances et dit si devant on a quelque chose � 2cm, et change la valeur de position_direction.way_ahead_state
	 if (get_prox(RIGHT_SIDE) > OBSTACLE_DISTANCE) {
		 position_direction.way_right_side_state=JAMMED;
	    }
		else {
			position_direction.way_right_side_state=FREE;
		}
}

void is_there_obstacle_left_side(void){
	//lit les distances et dit si devant on a quelque chose � 2cm, et change la valeur de position_direction.way_ahead_state
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


void go_round_the_inside(void){
	save_data_prior_avoiding();
	while(position_direction.status==AVOIDING &&(!avoiding_done())){
		is_there_obstacle_right_side();
		if(position_direction.way_right_side_state==1){
			move_turn(90,3);
			move_forward(3,5);
			continue;
		}
		is_there_obstacle_ahead();
		if(position_direction.way_ahead_state==1){
			move_forward(3,5);
			continue;
		}
		is_there_obstacle_left_side();
		if(position_direction.way_left_side_state==1){
			move_turn(90,-3);
			move_forward(3,5);
			continue;
		}
		move_turn(180,3);
	}
	position_direction.status=CRUISING;
}

float get_goal_distance(){
	if ((position_direction.current_direction==UP) || (position_direction.current_direction==RIGHT)){
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
    //position_direction.status==AVOIDING;
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	right_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);
	left_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);

	while ((right_motor_get_pos() < distance* STEPS_WHEEL_TURN / WHEEL_PERIMETER)
														&&(position_direction.status==CRUISING)){

	}
	halt();
	if(position_direction.status==AVOIDING){
		update_coordinate(right_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN);
		go_round_the_inside();
		return;
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
	//coordony update
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
	position_direction.current_position[0]=0;
	position_direction.current_position[1]=0;


}
void save_data_prior_avoiding (void){
	if((position_direction.current_direction==UP)||(position_direction.current_direction==DOWN)){
		position_direction.x_prior_avoiding=position_direction.current_position[0];
		position_direction.y_prior_avoiding=127;
	}
	else{
		position_direction.y_prior_avoiding=position_direction.current_position[1];
		position_direction.x_prior_avoiding=127;
	}
	position_direction.direction_prior_avoiding=position_direction.current_direction;


}
int avoiding_done(void){
	//de retour sur le bon x/y et la bonne position
	uint8_t done;
	uint8_t half_done;
	if((position_direction.x_prior_avoiding==127)&&
			(position_direction.y_prior_avoiding=position_direction.current_position[1])){
		half_done=1;
	}
	if((position_direction.y_prior_avoiding==127)&&
				(position_direction.x_prior_avoiding=position_direction.current_position[0])){
			half_done=1;
		}
	switch(position_direction.direction_prior_avoiding){
	case UP :
		if((position_direction.current_direction==RIGHT)&&(half_done)){
			done=1;
		}
		break;
	case DOWN:
		if((position_direction.current_direction==LEFT)&&(half_done)){
					done=1;
				}
		break;
	case LEFT :
		if((position_direction.current_direction==UP)&&(half_done)){
					done=1;
				}
		break;
	case RIGHT:
		if((position_direction.current_direction==DOWN)&&(half_done)){
					done=1;
				}
		break;

	}
	return done;
}
