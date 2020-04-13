#include "msgbus/messagebus.h"
#include "camera/dcmi_camera.h"
#include "parameter/parameter.h"

#define FRONT_RIGHT17 0
#define FRONT_RIGHT49 1
#define RIGHT 2
#define BACK_RIGHT 3
#define BACK_LEFT 4
#define LEFT 5
#define FRONT_LEFT49 6
#define FRONT_LEFT17 7


extern messagebus_t bus;











//constants for the differents parts of the project
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define MIN_LINE_WIDTH			40
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			2
#define PXTOCM					1570.0f //experimental value
#define GOAL_DISTANCE 			10.0f
#define MAX_DISTANCE 			25.0f

