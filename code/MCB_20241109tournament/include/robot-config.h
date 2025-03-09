using namespace vex;

extern brain Brain;

// VEXcode devices
extern controller Controller1;
extern motor br;
extern motor fr;
extern motor bl;
extern motor fl;
extern motor mr;
extern motor ml;
extern motor intak;
extern digital_out tilt;
extern motor Lift;
extern optical ringColorSensor;
extern digital_out elevation;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void  vexcodeInit( void );