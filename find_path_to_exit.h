#define DISTANCE_DESCRIBING_RIGHT_WALL           120  //experimental value
#define DISTANCE_DESCRIBING_FORWARD_WALL			250  //experimental value
#define DISTANCE_DESCRIBING_LEFT_WALL			90   //experimental value
#define LABIRYNTHE_CELL_SIZE						9.5f //The labyrinth is composed of equals cell sizes
#define MAX_SUM_ERROR							(MOTOR_SPEED_LIMIT/KI)
#define ERROR_THRESHOLD							150
#define PROXIMITY_DISTANCE_GOAL					1000
#define KP										800
#define KI										3.5
#define ANGLE_CORRECTION							2
#define THRESHOLD_RIGHT_WALL_THICKNESS			20
#define FREQUENCY_THREAD_FIND_PATH_TO_EXIT		20 //20hz
#define PERIOD_SLEEP_THREAD_FIND_PATH_TO_EXIT	(1000/FREQUENCY_THREAD_FIND_PATH_TO_EXIT) //in ms

typedef enum{
	too_close, too_far, perfect
}distance;

void find_path_to_exit_start(void);
void reset_counters_backwarding(void);
bool get_front_wall(void);
void set_front_wall(bool boolean);
