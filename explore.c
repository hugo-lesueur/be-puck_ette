#include <explore.h>

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
	}desired_direction, current_direction,future_direction; //desired: side to which we want to turn, futur: a temp variable that enables to

	enum {
		YES=1,
		NO=0
	}flee; //define if we need to return home or not --> update by the microphone

	enum {
		CRUISING=1,
		AVOIDING=2,
	}status; //define if we need to avoid an obstacle --> update by the IRs

	enum{
		FORWARD=1,
		TURNING=2
	}action; //define which action we are currently doing in the spirale

}position_direction;
//-----------------------------------------------THREADS--------------------------------------------------------------------


static THD_WORKING_AREA(waMove, 128);
static THD_FUNCTION(Move, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
    	if ((position_direction.status==CRUISING) &&(position_direction.flee==NO)){
			if (position_direction.action==FORWARD){
				distance_to_walk=get_goal_distance();
				move_forward((distance_to_walk-position_direction.progression), SPEED);
				position_direction.action=TURNING;
			}
			else if(position_direction.flee!=YES){
				move_turn(90,5);
				update_direction();
				position_direction.action=CRUISING;
				position_direction.progression=0;
			}
    	}
    	if((position_direction.status==AVOIDING)) {
    		go_round_the_inside();
    	}
    	if ((position_direction.flee==YES) && (position_direction.status==CRUISING)){
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
    		position_direction.status=CRUISING;

    		break;
    	case JAMMED:
    		position_direction.status=AVOIDING;
    		break;
    	}

    }
}

void ObstacleInspector_start(void){
	chThdCreateStatic(waObstacleInspector, sizeof(waObstacleInspector), NORMALPRIO, ObstacleInspector, NULL);
}

//-----------------------------------------------INTERNAL FUNCTIONS---------------------------------------------------------

void is_there_obstacle_ahead(void){
	//look if we have something ahead, change position_direction.way_ahead_state if we detect an obstacle
	 if ((get_prox(FRONT_LEFT) > OBSTACLE_DISTANCE) ||
		    (get_prox(FRONT_RIGHT) > OBSTACLE_DISTANCE)) {
		 position_direction.way_ahead_state=JAMMED;
	    }
		else {
			position_direction.way_ahead_state=FREE;
		}
	}


void is_there_obstacle_right_side(void){
	//look if we have something on the right side, change position_direction.right_side_state if we detect an obstacle
	 if (get_prox(RIGHT_SIDE) > OBSTACLE_DISTANCE) {
		 position_direction.way_right_side_state=JAMMED;
	    }
		else {
			position_direction.way_right_side_state=FREE;
		}
}


void is_there_obstacle_left_side(void){
	//look if we have something on the left side, change position_direction.way_left_side_state if we detect an obstacle
	 if (get_prox(LEFT_SIDE) > OBSTACLE_DISTANCE) {
		 position_direction.way_left_side_state=JAMMED;
	    }
		else {
			position_direction.way_left_side_state=FREE;
		}
	}


void RTH(void){ //return in the middle of the spirale
	rotate_right_direction_y();//looking up or down
	motor_reboot();
	move(SPEED+2);
	while ((right_motor_get_pos() < abs(position_direction.current_position[1])* STEPS_WHEEL_TURN / WHEEL_PERIMETER) && (position_direction.status==CRUISING)){
		chThdSleepMilliseconds(10);
	}//go to y=0
	halt();
	rotate_right_direction_x();

	motor_reboot();
	move(SPEED+2);
	while ((right_motor_get_pos() < abs(position_direction.current_position[0])* STEPS_WHEEL_TURN / WHEEL_PERIMETER) && (position_direction.status==CRUISING)){
		chThdSleepMilliseconds(10);//go to x=0
	}
	halt();
	clear_leds();
	set_body_led(0);
	chThdSleepSeconds(5);
	init_position_direction();
	return;
}



void rotate_right_direction_y(void){
	if ((position_direction.current_position[0]>0)&&(position_direction.current_position[1]>0)){
		if(position_direction.current_direction==UP)move_turn(180,SPEED);
		if(position_direction.current_direction==LEFT)move_turn(90,SPEED);//était à l'envers
		position_direction.current_direction=DOWN;
		return;
	}
	if ((position_direction.current_position[0]>0)&&(position_direction.current_position[1]<=0)){
		if(position_direction.current_direction==RIGHT)move_turn(90,SPEED);
		position_direction.current_direction=UP;
		return;
	}
	if ((position_direction.current_position[0]<=0)&&(position_direction.current_position[1]<=0)){
		if(position_direction.current_direction==DOWN)move_turn(180,SPEED);
		if(position_direction.current_direction==RIGHT)move_turn(90,SPEED);
		position_direction.current_direction=UP;
		return;
	}
	if ((position_direction.current_position[0]<=0)&&(position_direction.current_position[1]>0)){
		if(position_direction.current_direction==LEFT)move_turn(90,SPEED);
		position_direction.current_direction=DOWN;
		return;
	}
}


void rotate_right_direction_x(void){ // only is up or down
	if (position_direction.current_position[0]>0){
		if(position_direction.current_direction==UP)move_turn(90,SPEED);
		if(position_direction.current_direction==DOWN)move_turn(90,-SPEED);
		position_direction.current_direction=LEFT;
		return;
	}
	else{
		if(position_direction.current_direction==UP)move_turn(90,-SPEED);
		if(position_direction.current_direction==DOWN)move_turn(90,SPEED);
		position_direction.current_direction=RIGHT;
		return;
	}
}


void go_round_the_inside(void){			  //avoid obstacle
	position_direction.progression=left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;//keep in mind what we have already walked so we
	//don't walk it twice
	is_there_obstacle_left_side();
	if(position_direction.way_left_side_state==FREE){
		position_direction.desired_direction=LEFT;
		avoid_obstacle();
		return;
	}
	is_there_obstacle_right_side();
	if(position_direction.way_right_side_state==FREE){
		position_direction.desired_direction=RIGHT;
		avoid_obstacle();
		return;
	}
	is_there_obstacle_ahead();
	if(position_direction.way_ahead_state==FREE){
		move_forward(3,SPEED);
		return;
	}
}

void avoid_obstacle(void){


//-------------------------------turn to the left-------------------------------------
	position_direction.future_direction=LEFT;
	move_turn(90,SPEED);
	change_direction();
//-------------------------------go to the end of the obstacle------------------------
	motor_reboot();
	move(SPEED);
	while(get_prox(RIGHT_SIDE)>OBSTACLE_DISTANCE){
		chThdSleepMilliseconds(10);
	}
	position_direction.digression=left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;
	update_coordinate(left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN);
	move_forward(1.5*EPUCK_RADIUS, SPEED);
//-------------------------------face back front and avoid-----------------------------
	position_direction.future_direction=RIGHT;
	move_turn(90,-SPEED);
	change_direction();
	move_forward(2*EPUCK_RADIUS, SPEED);
	position_direction.progression=position_direction.progression+left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;
	motor_reboot();
	move(SPEED);
	while(get_prox(RIGHT_SIDE)>OBSTACLE_DISTANCE/2){
		chThdSleepMilliseconds(10);
	};
	position_direction.progression=position_direction.progression+left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;
	update_coordinate(left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN);
	move_forward(EPUCK_RADIUS, SPEED);
	position_direction.progression=position_direction.progression+left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN;

//-------------------------------go back to spiral trajectory-------------------------
	position_direction.future_direction=RIGHT;
	move_turn(90,-SPEED);
	change_direction();
	move_forward(1.5*EPUCK_RADIUS,SPEED);
	move_forward(position_direction.digression,SPEED);
//-------------------------------get back to spiral orientation-----------------------
	position_direction.future_direction=LEFT;
	move_turn(90,SPEED);
	change_direction();

	position_direction.status=CRUISING;
	position_direction.digression=0;

}

float get_goal_distance(){
	if ((position_direction.current_direction==UP) || (position_direction.current_direction==DOWN)){
		distance_to_walk+=distance_unit;
	}
	return distance_to_walk;
}



void change_direction(void){
	if (position_direction.future_direction==LEFT){
		++position_direction.current_direction;
		if(position_direction.current_direction ==5){
			position_direction.current_direction=1;
		}
	}
	if (position_direction.future_direction==RIGHT){
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



void motor_reboot(void){ //reset the motor position

	left_motor_set_pos(0);
	right_motor_set_pos(0);
}


void move_forward(float distance, float speed) //move a certain distance at a certain speed
{
	if(distance<0){
		return;
	}

	motor_reboot();

	move(speed);

	while ((right_motor_get_pos() < distance* STEPS_WHEEL_TURN / WHEEL_PERIMETER) && (position_direction.status==CRUISING) && (position_direction.flee==NO)){
		chThdSleepMilliseconds(10);
	}
	halt();

	update_coordinate(left_motor_get_pos()*WHEEL_PERIMETER/STEPS_WHEEL_TURN);

}


void halt (void){  //stop the e-puck
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



void init_position_direction(void){ //init our state variable
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
