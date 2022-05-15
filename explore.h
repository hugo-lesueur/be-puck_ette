#ifndef EXPLORE_H
#define EXPLORE_H
#define FALSE             0
#define TRUE              1
#define JAMMED		      0
#define FREE              1
#define FRONT_LEFT        7
#define FRONT_RIGHT       0
#define RIGHT_SIDE        2
#define LEFT_SIDE         5
#define OBSTACLE_DISTANCE 150
#define CORRECTION_FACTOR 1.296    // correct the angle of rotation to be more precise
#define CORRECTION_FORWARD /*0.95*/ 1 //correct the distance
#define VITESSE 8

#define WHEEL_AXIS_WIDTH  5.35f                  //[cm]
#define PERIMETER_EPUCK   (PI * WHEEL_AXIS_WIDTH)
#define WHEEL_PERIMETER   13                     // [cm]
#define FULL_TURN_DEGREES 360.0f                 // degrees for a whole revolution
#define STEPS_WHEEL_TURN  1000					 //number of steps per wheel revolution
#define EPUCK_RADIUS      4     				 //cm
#include <stdlib.h>// aaaaaaaaaaaaaaaaaaaaaaaaattentioooooooooooooooooooooooooooooooooooooon c'est chelou de devoir faire ça nan?
#include <stdio.h>
//---------------------------------Initialisation Threads---------------------------------------------
void Move_start(void);
void ObstacleInspector_start(void);
void init_position_direction(void);

//---------------------------------Detection---------------------------------------------
void is_there_obstacle_ahead(void);
void is_there_obstacle_right_side(void);
void is_there_obstacle_left_side(void);


//---------------------------------Movement---------------------------------------------
void go_round_the_inside(void);
void RTH(void);
float get_goal_distance(void);
void change_direction(void);
void motor_set_position(float position_r, float position_l, float speed_r, float speed_l);
void halt (void);
void move_forward(float distance, float speed);
void update_coordinate (int32_t distance);
void move_turn(float position, float speed);
void motor_reboot(void);
void move(float speed);
void avoid_obstacle(void);
void rotate_right_direction_y(void);
void rotate_right_direction_x(void);
void update_direction(void);
void set_to_flee (void);

//---------------------------------------LEDs--------------------------------------------
void led_update(void);


#endif /* EXPLORE_H*/
