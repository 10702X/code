#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
controller Controller1 = controller(primary);
motor tr = motor(PORT7, ratio6_1, false);
motor fr = motor(PORT8, ratio6_1, true);
motor bl = motor(PORT15, ratio6_1, true);
motor fl = motor(PORT11, ratio6_1, false);
motor br = motor(PORT10, ratio6_1, false);
motor tl = motor(PORT2, ratio6_1, true);
motor intak = motor(PORT9, ratio6_1, true);
digital_out tilt = digital_out(Brain.ThreeWirePort.H);
digital_out elevation = digital_out(Brain.ThreeWirePort.F);
digital_out doink = digital_out(Brain.ThreeWirePort.G);
motor ringCatcher = motor(PORT4, ratio18_1, false);
motor Lift = motor(PORT1, ratio36_1, false);
rotation ringCatcherRotation = rotation(PORT17, false);
optical ringColorSensor = optical(PORT16);

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