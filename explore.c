#include <explore.h>
#include <stdio.h>
#include <stdlib.h>
#include "motors.h"
#include "sensors/proximity.h"



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
    		move_turn(90,3);//presque sûr que /4 et pas /2, /2 c'est faire demi tour
    	break;
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
    		break;
    	case JAMMED:
    		position_direction.status=AVOIDING; 										//commenter quand on aura les micros+les scénarios de faits
    		break;
//sempahore ici pour dire ok, le mouvement peut avoir lieu?

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
		 position_direction.way_ahead_state=FREE;
	    }
		else {
			position_direction.way_ahead_state=JAMMED;
		}
	}
void is_there_obstacle_right_side(void){
	//lit les distances et dit si devant on a quelque chose à 2cm, et change la valeur de position_direction.way_ahead_state
	 if (get_prox(RIGHT_SIDE) > OBSTACLE_DISTANCE) {
		 position_direction.way_right_side_state=FREE;
	    }
		else {
			position_direction.way_right_side_state=JAMMED;
		}
	}
void is_there_obstacle_left_side(void){
	//lit les distances et dit si devant on a quelque chose à 2cm, et change la valeur de position_direction.way_ahead_state
	 if (get_prox(LEFT_SIDE) > OBSTACLE_DISTANCE) {
		 position_direction.way_left_side_state=FREE;
	    }
		else {
			position_direction.way_left_side_state=JAMMED;
		}
	}
void run_away(void){
	//aller au centre, on sait que c'est [0,0]
}
void go_round_the_inside(void){////////////////faire ça////////424242424242

	/*tourner de 90 degrés vers la gauche
	avancer d'une certaine distance
	while(on n'est pas revenus){
	is_there_obstacle_right_side();
	if(position_direction.way_right_side_state){
	tourner de 90 degrés;
	avancer de x;
	}
	continue;
	is_there_obstacle_ahead();
	if(position_direction.way_ahead_state){
	avancer de x;
	}
	continue;
	is_there_obstacle_left_side();
	if(position_direction.way_left_side_state){
	tourner de 90 degrés
	avancer de x}*/


	}

float get_distance_cm(void){

	return distance_cm;
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

void moove_forward_turn(void){ //inutile?
//	//move 20cm forward at 5cm/s
//	get_goal_distance();
//    motor_set_position(distance_cm, distance_cm, 5, 5);
//    //while(motor_position_reached() != POSITION_REACHED);
//    //counterclockwise rotation of 90Â°
//    motor_set_position(PERIMETER_EPUCK/4, PERIMETER_EPUCK/4, 5, -5);
//    //while(motor_position_reached() != POSITION_REACHED);
//    change_direction();
}

void move_turn(float angle, float speed)//je pense c'est OK
{
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	right_motor_set_speed(-speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);
	left_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);


	 while ((abs(left_motor_get_pos()) < abs((angle/FULL_TURN_DEGREES)*STEPS_WHEEL_TURN))
	    	&& (abs(right_motor_get_pos()) < abs((angle/FULL_TURN_DEGREES)*STEPS_WHEEL_TURN))) {
		}
	 halt();
	position_direction.action=1;
}


void move_forward(float distance, float speed)
{

	right_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);
	left_motor_set_speed(speed * STEPS_WHEEL_TURN / WHEEL_PERIMETER);

	left_motor_set_pos(0);
	right_motor_set_pos(0);

	while ((right_motor_get_pos() < distance* 1000 / WHEEL_PERIMETER)
														&&(position_direction.status==CRUISING)){

	}
	if(position_direction.status==AVOIDING){
		go_round_the_inside();
	}
	else{
		position_direction.action=2;
	}
	halt();
}

void halt (void){
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}


