#include <explore.h>
#include <stdio.h>
#include <stdlib.h>
#include "motors.h"

#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)


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

}position_direction;
//-----------------------------------------------THREADS--------------------------------------------------------------------



static THD_FUNCTION(ObstacleInspector, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
    	is_there_obstacle_ahead();

    	switch(position_direction.way_ahead_state){
    	case FREE:
    		break;//continue d'avancer, mais en vrai il faudrait faire qq chose pour voir que c'est OK
    	case JAMMED:
    		//test_call();
    		run_away(); 										//commenter quand on aura les micros+les scénarios de faits
    		break;

    	}

    }
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
void go_round_the_inside(void){

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
	if ((position_direction.current_direction==1) || (position_direction.current_direction==3)){
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

void moove_forward_turn(void){
	//move 20cm forward at 5cm/s
	get_goal_distance();
    motor_set_position(distance_cm, distance_cm, 5, 5);
    while(motor_position_reached() != POSITION_REACHED);
    //counterclockwise rotation of 90Â°
    motor_set_position(PERIMETER_EPUCK/4, PERIMETER_EPUCK/4, 5, -5);
    while(motor_position_reached() != POSITION_REACHED);
    change_direction();
}




