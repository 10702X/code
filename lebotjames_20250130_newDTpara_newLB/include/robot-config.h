using namespace vex;

extern brain Brain;

// VEXcode devices
extern controller Controller1;
extern motor tr;
extern motor fr;
extern motor bl;
extern motor fl;
extern motor br;
extern motor tl;
extern motor intak;
extern digital_out tilt;
extern digital_out elevation;
extern digital_out doink;
extern motor ringCatcher;
extern motor Lift;
extern rotation ringCatcherRotation;
extern optical ringColorSensor;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void  vexcodeInit( void );