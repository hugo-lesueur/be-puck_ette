#include <explore.h>
#include <stdio.h>
#include <stdlib.h>
#include "motors.h"
#include "sensors/proximity.h"
#include <math.h>
#include "arm_math.h"
#include "leds.h"



//-------------------------SEMAPHORE---------------------------------



//-----------------------------------------------STATIC VARIABLES-----------------------------------------------------------
static float distance_unit = 5;
static float distance_to_walk=0;


static struct position_direction{

	int32_t current_position [2];
	uint8_t way_ahead_state;
	uint8_t way_right_side_state;
	uint8_t way_left_side_state;
	uint8_t orientation_realtive_direction;				//represents the difference between direction and orientation
														//:1 is turned right, -1 is left, 2 is back
	int32_t digression; //distance de l'eloignement par rapport a la trajectoire prevue
	int32_t progression;

	enum {
		UP=1,
		LEFT=2,
		DOWN=3,
		RIGHT=4
	}desired_direction, current_direction,futur_direction; //desired: side to which we want to turn, futur: a temp variable that enables to

	enum {
		ON=1,
		OFF=0
	}track;

	enum {
		YES=1,
		NO=0
	}flee;

	enum {
		CRUISING=1,
		AVOIDING=2,
	}status;

	enum{
		FORWARD=1,
		TURNING=2
	}action;

}position_direction;
//-----------------------------------------------THREADS--------------------------------------------------------------------


static THD_WORKING_AREA(waMove, 128);
static THD_FUNCTION(Move, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
    	//led_update();
    	if ((position_direction.status==CRUISING) &&(position_direction.flee==NO)){
			if (position_direction.action==FORWARD){
				if(position_direction.current_direction==UP){set_led(LED1,100);}
				if(position_direction.current_direction==DOWN){set_led(LED1,0);}
				if(position_direction.current_direction==RIGHT){set_led(LED5,100);}
				if(position_direction.current_direction==LEFT){set_led(LED5,0);}
				distance_to_walk=get_goal_distance();
				move_forward((distance_to_walk-position_direction.progression), 6);
				position_direction.action=TURNING;
			}
			else if(position_direction.flee!=YES){
				move_turn(90,5);
				update_direction();
				position_direction.action=CRUISING;
				position_direction.progression=0;
			}
    	}
    	if((position_direction.status==AVOIDING)&&(position_direction.flee==NO)) {
    		//position_direction.progression=0; pas de sens parcequ'on a besoin des coups d'avant, au cas o� y a deux obstacles sur la m�me branche de la spirale
    		go_round_the_inside();
    	}
    	if (position_direction.flee==YES){
    		RTH();
    	}
    }
}

void Move_start(void){
	chThdCreateStatic(waMove, sizeof(waMove), NORMALPRIO, Move, NULL);
}



static THD_WORKING_AREA(waObstacleInspector, 128);
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


void RTH(void){ //retourner en zone "sure"
	rotate_right_direction_y();
	move_forward(abs(position_direction.current_position[1]),6);
	rotate_right_direction_x();
	move_forward(abs(position_direction.current_position[0]),6);
	clear_leds();
	set_body_led(0);
	init_position_direction();
	//fonction �teindre les threads
	return;
}



void rotate_right_direction_y(void){
	if ((position_direction.current_position[0]>0)&&(position_direction.current_position[1]>0)){
		if(position_direction.current_direction==UP)move_turn(180,6);
		if(position_direction.current_direction==LEFT)move_turn(90,-6);
		if(position_direction.current_direction==RIGHT)move_turn(90,6);
		position_direction.current_direction=DOWN;
		return;
	}
	if ((position_direction.current_position[0]>0)&&(position_direction.current_position[1]<=0)){
		if(position_direction.current_direction==DOWN)move_turn(180,6);
		if(position_direction.current_direction==LEFT)move_turn(90,6);
		if(position_direction.current_direction==RIGHT)move_turn(90,-6);
		position_direction.current_direction=UP;
		return;
	}
	if ((position_direction.current_position[0]<=0)&&(position_direction.current_position[1]<=0)){
		if(position_direction.current_direction==DOWN)move_turn(180,6);
		if(position_direction.current_direction==LEFT)move_turn(90,-6);
		if(position_direction.current_direction==RIGHT)move_turn(90,6);
		position_direction.current_direction=UP;
		return;
	}
	if ((position_direction.current_position[0]<=0)&&(position_direction.current_position[1]>0)){
		if(position_direction.current_direction==UP)move_turn(180,6);
		if(position_direction.current_direction==LEFT)move_turn(90,6);
		if(position_direction.current_direction==RIGHT)move_turn(90,-6);
		position_direction.current_direction=DOWN;
		return;
	}
}


void rotate_right_direction_x(void){
	if (position_direction.current_position[0]>0){
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


void go_round_the_inside(void){			  //avoid obstacle
	position_direction.status=AVOIDING;   //not on track
	position_direction.progression=left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;//keep in mind what we have already walked so we
	//don't walk it twice
	is_there_obstacle_left_side();
	if(position_direction.way_left_side_state==FREE){
		position_direction.desired_direction=LEFT;
		avoid_obstacle();
		return;
	}//utile?
	is_there_obstacle_right_side();
	if(position_direction.way_right_side_state==FREE){
		position_direction.desired_direction=RIGHT;
		avoid_obstacle();
		return;
	}//pour quoi?
	is_there_obstacle_ahead();
	if(position_direction.way_ahead_state==FREE){
		move_forward(3,6);
		return;
	}
}

void avoid_obstacle(void){
//------------------------right side-------------------------
	if(position_direction.desired_direction==RIGHT){
		move_turn(90,-6);
		position_direction.futur_direction=RIGHT;
		change_direction();
		while(get_prox(LEFT_SIDE)>OBSTACLE_DISTANCE){
			position_direction.digression= left_motor_get_pos(); //retiens distance d'eloignement
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
		move_forward(position_direction.digression* STEPS_WHEEL_TURN / WHEEL_PERIMETER,6);
		move_turn(90,-6);
		position_direction.futur_direction=RIGHT;
		change_direction();
		position_direction.status=CRUISING;

	}



//--------------------------------LEFT SIDE-------------------------------------------
	if(position_direction.desired_direction==LEFT){
		position_direction.futur_direction=LEFT;
		//position_direction.progression=0; faut pas on doit pouvoir retirer au move_forward qui vient apr�s avoir contourn� une fois avant sur la branche si deux obstacles
		change_direction();//ici chelou
		move_turn(90,6);//first turn to be parallel
					if(position_direction.current_direction==UP){set_led(LED1,100);}
					if(position_direction.current_direction==DOWN){set_led(LED1,0);}
					if(position_direction.current_direction==RIGHT){set_led(LED5,100);}
					if(position_direction.current_direction==LEFT){set_led(LED5,0);}
		motor_reboot();
//-------------------------first slide------------------------------
		while(get_prox(RIGHT_SIDE)>OBSTACLE_DISTANCE){
			move(6);//get up to the end of the obstacle
		}
		position_direction.digression= left_motor_get_pos(); //retient distance d'eloignement
		//change_direction(); pourquoi c'est l� on n'a pas tourn�?
		move_forward(EPUCK_RADIUS,6); //advance to not hit the wall
		position_direction.futur_direction=RIGHT;
		change_direction();
		move_turn(90,-6);//turn to be perpendicular to the obstacle
					if(position_direction.current_direction==UP){set_body_led(1);}
					if(position_direction.current_direction==DOWN){set_led(LED1,0);}
					if(position_direction.current_direction==RIGHT){set_led(LED5,100);}
					if(position_direction.current_direction==LEFT){set_led(LED5,0);}
		move_forward(EPUCK_RADIUS*2,6); //go back up next to the obstacle
		position_direction.progression+=left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;//saves these 2 radii to progression
		motor_reboot();//not to add radius*2 again//on veut garder ce qu'on a avanc� pour le soustraire � la fin
//-----------------------side--------------------------------------
		move(6);
		while(get_prox(RIGHT_SIDE)>OBSTACLE_DISTANCE/2){

		}
		halt();
		position_direction.progression+=left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;//juste si move fait bien compter les tours de moteur
		move_forward(EPUCK_RADIUS,6); //advance to not hit the wall
		position_direction.progression+=left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;//maintenant on a tout ce dont on a avanc�
		//update_coordinate(position_direction.progression); //updated par move forward, fait plus de sens
		position_direction.futur_direction=RIGHT;
		change_direction();
		move_turn(90,-6);
					if(position_direction.current_direction==UP){set_led(LED1,100);}
					if(position_direction.current_direction==DOWN){set_led(LED1,0);}
					if(position_direction.current_direction==RIGHT){set_led(LED5,100);}
					if(position_direction.current_direction==LEFT){set_led(LED5,0);}
		move_forward(EPUCK_RADIUS,6); //advance to detect smth
//----------------comeback on right track--------------------------
		set_led(LED7,100);
		set_body_led(0);
		move_forward(position_direction.digression*WHEEL_PERIMETER*CORRECTION_FORWARD/STEPS_WHEEL_TURN ,6);//this is weird
		position_direction.futur_direction=LEFT;
		change_direction();
		move_turn(90,6);
					if(position_direction.current_direction==UP){set_led(LED1,100);}
					if(position_direction.current_direction==DOWN){set_led(LED1,0);}
					if(position_direction.current_direction==RIGHT){set_led(LED5,100);}
					if(position_direction.current_direction==LEFT){set_led(LED5,0);}
		position_direction.status=CRUISING;
		position_direction.digression=0;
	}

}

float get_goal_distance(){
	if ((position_direction.current_direction==UP) || (position_direction.current_direction==DOWN)){
		distance_to_walk+=distance_unit;
	}
	return distance_to_walk;
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
	if(distance<0){
		return;
	}

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
		//position_direction.progression+=left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN; non car se fait changer par tout le monde baah
		update_coordinate(left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN);
//	}

}

void halt (void){
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}

void update_coordinate (int32_t distance){
	//coordinate update
	if (position_direction.current_direction==UP){
		position_direction.current_position[1]=position_direction.current_position[1]+distance;
	}
	if (position_direction.current_direction==DOWN){
			position_direction.current_position[1]=position_direction.current_position[1]-distance;
	}
	if (position_direction.current_direction==RIGHT){
			position_direction.current_position[0] =position_direction.current_position[0]+distance;
	}
	if (position_direction.current_direction==LEFT){
		position_direction.current_position[0]=position_direction.current_position[0]-distance;
	}
}

void led_update(void){
	if ((position_direction.status==CRUISING) &&(position_direction.flee==NO)) {
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
	if (position_direction.flee==YES) {
		set_rgb_led(LED2,100,0,0);
		set_rgb_led(LED3,100,0,0);
		set_rgb_led(LED4,100,0,0);
		set_rgb_led(LED5,100,0,0);
	}
}

void init_position_direction(void){
	position_direction.current_position[0]=0;
	position_direction.current_position[1]=0;
	position_direction.current_direction=UP;
	position_direction.status=CRUISING;
	position_direction.action=FORWARD;
	position_direction.progression=0;
	position_direction.flee=NO;
	distance_to_walk=0;
	return;
}

void set_to_flee (void){
	position_direction.flee=YES;
}
void update_direction(void){
	if(position_direction.current_direction==RIGHT){
		position_direction.current_direction=UP;
	}
	else{
		position_direction.current_direction+=1;
	}

}
