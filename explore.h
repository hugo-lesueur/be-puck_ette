#ifndef EXPLORE_H
#define EXPLORE_H


#define JAMMED		      0
#define FREE              1
#define FRONT_LEFT        7
#define FRONT_RIGHT       0
#define RIGHT_SIDE        1
#define LEFT_SIDE         6
#define OBSTACLE_DISTANCE 300
#define CORRECTION_FACTOR           1.31    // correct the angle of rotation to be more precise

#define WHEEL_AXIS_WIDTH  5.35f                  //[cm]
#define PERIMETER_EPUCK   (PI * WHEEL_AXIS_WIDTH)
#define WHEEL_PERIMETER   13                     // [cm]
#define FULL_TURN_DEGREES 360.0f                   // degrees for a whole revolution
#define STEPS_WHEEL_TURN  1000					 //number of steps per wheel revolution

//---------------------------------Initialisation Threads---------------------------------------------
void Move_start(void);
void ObstacleInspector_start(void);

//---------------------------------Detection---------------------------------------------
void is_there_obstacle_ahead(void);
void is_there_obstacle_right_side(void);
void is_there_obstacle_left_side(void);


//---------------------------------Mouvement---------------------------------------------
void go_round_the_inside(void);
void run_away(void);
float get_goal_distance(void);
void change_direction(void);
void motor_set_position(float position_r, float position_l, float speed_r, float speed_l);
void halt (void);
void move_forward(float distance, float speed);
void update_coordinate (float distance);
void move_turn(float position, float speed);


#endif /* EXPLORE_H*/
