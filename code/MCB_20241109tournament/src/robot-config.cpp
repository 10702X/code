#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
controller Controller1 = controller(primary);
motor br = motor(PORT18, ratio6_1, false);
motor fr = motor(PORT19, ratio6_1, true);
motor bl = motor(PORT13, ratio6_1, true);
motor fl = motor(PORT11, ratio6_1, false);
motor mr = motor(PORT16, ratio6_1, true);
motor ml = motor(PORT14, ratio6_1, false);
motor intak = motor(PORT20, ratio6_1, true);
digital_out tilt = digital_out(Brain.ThreeWirePort.H);
motor Lift = motor(PORT15, ratio36_1, false);
optical ringColorSensor = optical(PORT17);
digital_out elevation = digital_out(Brain.ThreeWirePort.G);

// VEXcode generated functions
// define variable for remote controller enable/disable
bool RemoteControlCodeEnabled = true;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void vexcodeInit( void ) {
  // nothing to initialize
}