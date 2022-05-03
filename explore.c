#include <explore.h>
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------STATIC VARIABLES-----------------------------------------------------------
static struct position_direction{

	uint8_t current_position [2];
	uint8_t way_ahead_state;
	uint8_t way_right_side_state;
	uint8_t way_left_side_state;
	uint8_t orientation_realtive_direction;				//represents the difference between direction and orientation
														//:1 is turned right, -1 is left, 2 is back
	enum {
		UP=1,
		DOWN=2,
		RIGHT=3,
		LEFT=4

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





