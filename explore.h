#ifndef EXPLORE_H
#define EXPLORE_H


#define JAMMED		      0
#define FREE              1
#define FRONT_LEFT        7
#define FRONT_RIGHT       0
#define RIGHT_SIDE        1
#define LEFT_SIDE         6
#define OBSTACLE_DISTANCE 300


void is_there_obstacle_ahead(void);
void is_there_obstacle_right_side(void);
void is_there_obstacle_left_side(void);
void go_round_the_inside(void);
void run_away(void);
float get_distance_cm(void);
float get_goal_distance(void);
void change_direction(void);
void moove_forward_turn(void);

#endif /* OBSTACLE_AVOID_H */
