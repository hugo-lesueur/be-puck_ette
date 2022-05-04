#ifndef EXPLORE_H
#define EXPLORE_H


#define JAMMED		      0
#define FREE              1
#define FRONT_LEFT        7
#define FRONT_RIGHT       0
#define RIGHT_SIDE        1
#define LEFT_SIDE         6
#define OBSTACLE_DISTANCE 300

#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)
#define WHEEL_PERIMETER     13 // [cm]


void is_there_obstacle_ahead(void);
void is_there_obstacle_right_side(void);
void is_there_obstacle_left_side(void);
void go_round_the_inside(void);
void run_away(void);
float get_distance_cm(void);
float get_goal_distance(void);
void change_direction(void);
void moove_forward_turn(void);
void motor_set_position(float position_r, float position_l, float speed_r, float speed_l);
void halt (void);
void move_forward(float distance, float speed);

void move_turn(float position, float speed);

#endif /* OBSTACLE_AVOID_H */
