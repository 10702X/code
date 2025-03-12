#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// tr                   motor         7               
// fr                   motor         8               
// bl                   motor         15              
// fl                   motor         11              
// br                   motor         10              
// tl                   motor         2               
// intak                motor         9               
// tilt                 digital_out   H               
// elevation            digital_out   F               
// doink                digital_out   G               
// ringCatcher          motor         4               
// Lift                 motor         1               
// ringCatcherRotation  rotation      17              
// ringColorSensor      optical       12              
// ---- END VEXCODE CONFIGURED DEVICES ----


using namespace vex;

// A global instance of competition
competition Competition;

inertial Inertial = inertial(PORT13);
//vex::distance RingDistance = vex::distance(PORT1); //somehow, adding this to device list makes error
//vex::distance wallDistance = vex::distance(PORT10); //somehow, adding this to device list makes error

motor_group leftDrive (fl, tl, bl);
motor_group rightDrive (fr, tr, br);
smartdrive Chicken(leftDrive, rightDrive, Inertial, 10.205,10.75,11,distanceUnits::in);

/*---------------------------------------------------------------------------*/
/*                             VEXcode Config                                */
/*                                                                           */
/*  Before you do anything else, start by configuring your motors and        */
/*  sensors using the V5 port icon in the top right of the screen. Doing     */
/*  so will update robot-config.cpp and robot-config.h automatically, so     */
/*  you don't have to. Ensure that your motors are reversed properly. For    */
/*  the drive, spinning all motors forward should drive the robot forward.   */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                             JAR-Template Config                           */
/*                                                                           */
/*  Where all the magic happens. Follow the instructions below to input      */
/*  all the physical constants and values for your robot. You should         */
/*  already have configured your robot manually with the sidebar configurer. */
/*---------------------------------------------------------------------------*/

Drive chassis(

//Specify your drive setup below. There are seven options:
//ZERO_TRACKER_NO_ODOM, ZERO_TRACKER_ODOM, TANK_ONE_ENCODER, TANK_ONE_ROTATION, TANK_TWO_ENCODER, TANK_TWO_ROTATION, HOLONOMIC_TWO_ENCODER, and HOLONOMIC_TWO_ROTATION
//For example, if you are not using odometry, put ZERO_TRACKER_NO_ODOM below:
ZERO_TRACKER_NO_ODOM,

//Add the names of your Drive motors into the motor groups below, separated by commas, i.e. motor_group(Motor1,Motor2,Motor3).
//You will input whatever motor names you chose when you configured your robot using the sidebar configurer, they don't have to be "Motor1" and "Motor2".

//Left Motors:
motor_group(fl, tl, bl),

//Right Motors:
motor_group(fr, tr, br),

//Specify the PORT NUMBER of your inertial sensor, in PORT format (i.e. "PORT1", not simply "1"):
PORT13,

//Input your wheel diameter. (4" omnis are actually closer to 4.125"):
3.25,

//External ratio, must be in decimal, in the format of input teeth/output teeth.
//If your motor has an 84-tooth gear and your wheel has a 60-tooth gear, this value will be 1.4.
//If the motor drives the wheel directly, this value is 1:
0.6,

//Gyro scale, this is what your gyro reads when you spin the robot 360 degrees.
//For most cases 360 will do fine here, but this scale factor can be very helpful when precision is necessary.
360,

/*---------------------------------------------------------------------------*/
/*                                  PAUSE!                                   */
/*                                                                           */
/*  The rest of the drive constructor is for robots using POSITION TRACKING. */
/*  If you are not using position tracking, leave the rest of the values as  */
/*  they are.                                                                */
/*---------------------------------------------------------------------------*/

//If you are using ZERO_TRACKER_ODOM, you ONLY need to adjust the FORWARD TRACKER CENTER DISTANCE.

//FOR HOLONOMIC DRIVES ONLY: Input your drive motors by position. This is only necessary for holonomic drives, otherwise this section can be left alone.
//LF:      //RF:    
PORT1,     -PORT2,

//LB:      //RB: 
PORT3,     -PORT4,

//If you are using position tracking, this is the Forward Tracker port (the tracker which runs parallel to the direction of the chassis).
//If this is a rotation sensor, enter it in "PORT1" format, inputting the port below.
//If this is an encoder, enter the port as an integer. Triport A will be a "1", Triport B will be a "2", etc.
3,

//Input the Forward Tracker diameter (reverse it to make the direction switch):
2.75,

//Input Forward Tracker center distance (a positive distance corresponds to a tracker on the right side of the robot, negative is left.)
//For a zero tracker tank drive with odom, put the positive distance from the center of the robot to the right side of the drive.
//This distance is in inches:
-2,

//Input the Sideways Tracker Port, following the same steps as the Forward Tracker Port:
1,

//Sideways tracker diameter (reverse to make the direction switch):
-2.75,

//Sideways tracker center distance (positive distance is behind the center of the robot, negative is in front):
5.5

);

int current_color_selection = 0; //0: red; 1: blue; 
//bool auto_started = false; //for competition
bool auto_started = true; //for skills and practice
bool intakeStop = true;
bool outtakeStop = true;
//bool ringColorSorting = false; 
bool ringColorSorting = true; //use optical sensor to sort ring colors
bool ringColorCorrect = true;
bool fanOut = false;
bool loadingIntake = 0;
bool loading = 0; //not load the ring box
bool dunking = 0;

bool intakeRunning=false;
bool foundIntakeJam = false;
event checkIntake = event();
event ringColorCheck = event();

void printTemps() //print each motor temperature on the brain
{
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(1,0);
      Brain.Screen.print(" bl T=");
      Brain.Screen.print(fl.temperature(celsius));
      Brain.Screen.print(" fl T=");
      Brain.Screen.print(fl.temperature(celsius));

      Brain.Screen.newLine();
      Brain.Screen.print(" br T=");
      Brain.Screen.print(br.temperature(celsius));
      Brain.Screen.print("fr T=");
      Brain.Screen.print(fr.temperature(celsius));
      Brain.Screen.newLine();
      Brain.Screen.print(" Ring Catcher T=");
      Brain.Screen.print(ringCatcher.temperature(celsius));
      Brain.Screen.print(" intake T=");
      Brain.Screen.print(intak.temperature(celsius));
     
}//end of printTemps()

/**********************************************************************/
void runChassis (int leftSpeed, int rightSpeed)
{
  //set the wheel motor torque to max
  
  bl.setMaxTorque(100,pct);
  tl.setMaxTorque(100,pct);
  fl.setMaxTorque(100,pct);
  br.setMaxTorque(100,pct);
  tr.setMaxTorque(100,pct);
  fr.setMaxTorque(100,pct);
  leftDrive.spin(fwd, leftSpeed, pct);
  rightDrive.spin(fwd, rightSpeed, pct);
}// end of runChassis

void frontBrake()
{
  bl.stop(coast);
  br.stop(coast);
  tl.stop(coast);
  tr.stop(coast);
  fl.stop(brake);
  fr.stop(brake);
} //end of frontBrake()

void rearBrake()
{
  bl.stop(brake);
  br.stop(brake);
  tl.stop(brake);
  tr.stop(brake);
  fl.stop(coast);
  fr.stop(coast);
}//end of rearBrake()

void stopBrake()
{
  bl.stop(hold);
  br.stop(hold);
  tl.stop(hold);
  tr.stop(hold);
  fl.stop(hold);
  fr.stop(hold);
}//end of stopBrake()



void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  Brain.Screen.setFont(vex::fontType::mono60);

  vexcodeInit();
  Inertial.calibrate();
  default_constants();
  tilt.set(false);
  ringCatcher.setPosition(0, degrees);
  ringCatcher.setVelocity(100,percent);
  ringCatcher.setBrake(hold);
  ringCatcherRotation.setPosition(0, degrees);

  // color selection on the brain screen
  //Tap the brain screen to choose allance color.
  while(auto_started == false){            
    Brain.Screen.clearScreen();
    switch(current_color_selection){       
      case 0:
        Brain.Screen.printAt(100, 100, "Red");
        Controller1.Screen.print("Red");
        break;
      case 1:
        Brain.Screen.printAt(100, 100, "Blue");
        Controller1.Screen.print("Blue");
        break;
    }
    if(Brain.Screen.pressing()){
      while(Brain.Screen.pressing()) {}
      current_color_selection ++;
    } else if (current_color_selection == 2){
      current_color_selection = 0;
    }
    task::sleep(10);
  } //while
 // cout<<"end of pre auton\n";
} //end of pre_auton


/*********use optical and distance sensors to sort rings*************/
// when the optical sensor detects rings with !current_color_section
// intake stops for 0.4 sec so the ring will go out
// otherwise intake will keep running and put the correct color ring into the goal
/*void sortingIntake()
{

  color detectedColor = ringColorSensor.color();
  intak.spin(fwd,100,velocityUnits::pct);
 cout<<"color="<<detectedColor<<" distance="<<RingDistance.objectDistance(mm)<<"\n";
  if(current_color_selection==0) //red alliance
  {//if blue ring is detected and in position
    if(RingDistance.objectDistance(mm) < 37 && detectedColor == blue)
    {
      intak.stop(brakeType::hold); 
      //stop intake for 0.4 second so the blue ring will fall out
      wait(0.4,sec);
      intak.spin(fwd,100,velocityUnits::pct); //start the intake again
      ringColorSensor.setLight(ledState::off);
    }
  }//red alliance
  else if (current_color_selection==1) //blue alliance
  {
    //if red ring is detected and in position
    if(RingDistance.objectDistance(mm) < 37 && detectedColor == red)
    {
      intak.stop(brakeType::hold); 
      //stop intake for 0.4 second so the blue ring will fall out
      wait(0.4,sec);
      intak.spin(fwd,100,velocityUnits::pct); //start the intake again
      ringColorSensor.setLight(ledState::off);
    }
  }
}*/ //end of sortingIntake

/**************************************/
void dunk()
{
  dunking = 1;
  double target = -135.0;
  double current =0.0;
  double error =target-current;
  double preverror = error;
  double p=0.2,d=0.1;
  double delerror =preverror-error;
  int count=0;
  cout<<"start dunking\n";
  while(abs(error)>=0.2 && !loading){
    current = ringCatcherRotation.position(degrees);
    cout<<"curr: "<<current<<endl;
    error = target - current;
    cout<<"error: "<<error<<endl;
    delerror =preverror - error;;
    float nextvolt = abs(error) * p + delerror * d;
    cout<<"nextvolt="<<nextvolt<<endl;
    if(nextvolt>12) {nextvolt = 12;}
    if(nextvolt<2) {nextvolt = 2;}
    
    if (error<0) { ringCatcher.spin(fwd,nextvolt,voltageUnits::volt); }
    else { ringCatcher.spin(reverse,nextvolt,voltageUnits::volt); }
    preverror = error;
    wait(0.01,sec);
    count++;
  }
  dunking = 0;
  ringCatcher.stop();
  current = ringCatcherRotation.position(degrees);
  cout<<"end: "<<current<<" count = "<<count<<endl;
} // end of dunk()

/***********************************/
//solo program to get AWP
void AWP_solo()
{
  rearBrake();
  chassis.drive_timeout = 500;
  chassis.drive_distance(-16);
  chassis.drive_distance(-9, 0, 6, 6, 1, 300, 500);
  tilt.set(true); //clamp on the goal
  wait(0.3,sec); 

  //go get the second ring and score onto the goal
  chassis.turn_max_voltage = 6;
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(100);
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 1000;
  chassis.drive_distance(22);//18ogadded4
  wait(0.8,sec);
  
  //turn, drop the goal, and go to the other side
  chassis.drive_timeout = 1000;
  chassis.turn_timeout = 1000;
  chassis.turn_to_angle(293); //og297
  wait(0.05,sec);
  intak.stop();
  tilt.set(false);

  chassis.drive_distance(40);//og31
  //spit blue ring and get red ring
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_max_voltage = 4;
  chassis.drive_distance(10);
  wait(0.05,sec);
  chassis.drive_max_voltage = 8;
  chassis.drive_distance(14,293,4,0,1,300,600);
  intak.stop();
  //get the next mobile goal
  chassis.turn_to_angle(32);
  chassis.drive_distance(-26,32,12,12,1,300,700);
  chassis.drive_distance(-15, 32, 5, 6, 1, 300, 500);
  tilt.set(true);
  wait(0.03,sec);
  //get the last ring
  chassis.turn_to_angle(270);
  intak.spin(fwd,10,voltageUnits::volt);
  chassis.drive_max_voltage = 10;
  chassis.drive_distance(28);
  //touch the bar
  chassis.drive_max_voltage = 12;
  intak.stop();
  chassis.turn_max_voltage = 12; //
  chassis.turn_to_angle(105);
  intak.spin(fwd,12,voltageUnits::volt);
  dunk();
  chassis.drive_timeout = 2000;
  chassis.drive_distance(27);
  //intak.stop();
}//end of AWP_solo 

void middlegoal_qual()
{

  if(current_color_selection==0)//red
  {
    rearBrake();
    chassis.drive_distance(-16, 0, 6, 6, 1, 300, 500);
    chassis.drive_distance(-13, 0, 4, 6, 1, 300, 600);
    tilt.set(true); //clamp on the goal
    wait(0.3,sec); 

    //score the preload onto the goal
    intak.spin(fwd,12,voltageUnits::volt);
    wait(0.5,sec);
  //  intak.stop();

    //go get the second ring and store in intake
    chassis.turn_max_voltage = 6;
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(260);
    tilt.set(false); //release first goal
    intak.spin(fwd,9,voltageUnits::volt);
    wait(0.1,sec);
    chassis.drive_max_voltage = 12;
    chassis.drive_timeout = 1000;
    chassis.drive_distance(27.4);
    intak.stop();
  
    //get the middle goal
    chassis.turn_max_voltage = 4;
    chassis.turn_timeout = 1000;
    chassis.turn_to_angle(0);
    chassis.drive_distance(-14,0,6,6,1,300,600);
    chassis.drive_distance(-7.5, 0, 4, 6, 1, 300, 410);
    tilt.set(true);
    wait(0.3,sec);
    //put ring on goal
    intak.spin(fwd,12,voltageUnits::volt);

    //touch the ladder
    chassis.drive_distance(30);
    chassis.turn_timeout = 1000;
    chassis.turn_to_angle(130);
    dunk();
    intak.stop();
    chassis.drive_distance(29, 130, 10, 6, 1, 300, 1500);
    
  //go clear corner
/*  chassis.drive_max_voltage = 10;
  chassis.turn_max_voltage = 7;
  chassis.drive_timeout = 900;
  chassis.drive_distance(30);
  intak.stop();
  chassis.drive_distance(15);
  chassis.drive_distance(-15);
  
 // wait(0.2,sec);
  chassis.turn_to_angle(333);
  doink.set(true); //extend the fan
  chassis.drive_distance(22);
  chassis.turn_to_angle(320);
  chassis.drive_distance(5);
  chassis.turn_to_angle(220);
  */
  }
  else //blue alliance
  {
    rearBrake();
    chassis.drive_timeout = 500;
   chassis.drive_distance(-16);
    chassis.drive_distance(-9, 0, 4, 6, 1, 300, 600);
    tilt.set(true); //clamp on the goal
    wait(0.3,sec); 
  //chassis.drive_distance(3, 0, 6, 6, 1, 300, 500);

  //score the preload onto the goal
    intak.spin(fwd,12,voltageUnits::volt);

  //go get the second ring and store in intake
    chassis.turn_max_voltage = 6;
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(100);
    tilt.set(false); //release first goal
    intak.spin(fwd,9,voltageUnits::volt);
    wait(0.1,sec);
    chassis.drive_max_voltage = 12;
    chassis.drive_timeout = 1000;
    chassis.drive_distance(27.4);
    intak.stop();
    chassis.drive_distance(-2);
    //get the middle goal
    chassis.turn_max_voltage = 4;
    chassis.turn_to_angle(0);
    chassis.drive_timeout = 600;
    chassis.drive_max_voltage = 6;
    chassis.drive_distance(-14);
    chassis.drive_distance(-7.5, 0, 4, 6, 1, 300, 410);
    tilt.set(true);
    wait(0.3,sec);
  //put ring on goal
    intak.spin(fwd,12,voltageUnits::volt);
    chassis.drive_max_voltage = 10;
    chassis.turn_max_voltage = 7;
    chassis.drive_timeout = 900;
    chassis.drive_distance(30);
 
    //touch the ladder
    chassis.turn_to_angle(240);
    dunk();
    intak.stop();
    chassis.drive_distance(32, 240, 10, 6, 1, 300, 1500);
  /*//go clear corner
    chassis.drive_max_voltage = 10;
    chassis.turn_max_voltage = 7;
    chassis.drive_timeout = 900;
    chassis.drive_distance(30);
    intak.stop();
    wait(0.2,sec);
    chassis.turn_to_angle(27);
    doink.set(true); //extend the fan
    chassis.drive_distance(22);
    chassis.turn_to_angle(40);
    chassis.drive_distance(5);
    chassis.turn_to_angle(140);
    doink.set(false);
    */
  }
}
void fourRings_AWP()
{
  if(current_color_selection==0)//red
  {
    chassis.turn_timeout =300;
    chassis.turn_to_angle(16);
    dunk(); // put preload onto the alliance stake
    ringCatcher.spinToPosition(0,degrees);

    chassis.drive_distance(-30,90,12,9.5, 1,300,1200);
    chassis.drive_distance(-14, 90, 4, 6,1,300,800);
    tilt.set(true);
    wait(0.3,sec);

    chassis.turn_max_voltage = 6;
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(190);
  
    intak.spin(fwd,12,voltageUnits::volt);

    chassis.drive_max_voltage = 12;
    chassis.drive_timeout = 1000;
    chassis.drive_distance(21);
    wait(0.5,sec);

    //go get the third ring(on the right) and score onto the goal
    chassis.drive_timeout = 1000;
    chassis.drive_distance(-14);
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(239);//og132
    chassis.drive_timeout = 800;
    chassis.drive_distance(20);
    wait(0.5,sec);

    //go get the 4th ring(on the left) and score onto the goal  
    chassis.drive_distance(-26,239,9,6);
    chassis.turn_timeout = 500;
    chassis.turn_to_angle(207);//100
    chassis.drive_timeout = 800;
    chassis.drive_distance(17);
    chassis.turn_to_angle(242);//130
    chassis.drive_distance(16);
    wait(0.5,sec);
    chassis.drive_distance(-23);
    chassis.turn_timeout = 500;

    //touch the ladder
    chassis.turn_to_angle(317);//210
    intak.stop();
    ringCatcher.spinToPosition(520,degrees,false);
    frontBrake();
    chassis.drive_distance(18, 317, 6, 6, 1, 300, 1500);//20
    
  } // end of red 
  else //blue
  {
    chassis.turn_timeout =300;
    chassis.turn_to_angle(344);
    dunk(); //put the preload onto the alliance stake
    ringCatcher.spinToPosition(0,degrees,false); //reset the ringCatcher

    //go get the goal
    chassis.drive_distance(-30,270,12,8.7,1,300,1600);
    chassis.drive_distance(-14, 270, 4, 6);
    tilt.set(true);
    wait(0.3,sec);

    chassis.turn_max_voltage = 6;
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(170);
  
    intak.spin(fwd,12,voltageUnits::volt);

    chassis.drive_max_voltage = 12;
    chassis.drive_timeout = 1000;
    chassis.drive_distance(21);
    wait(0.5,sec);

    //go get the third ring(on the right) and score onto the goal
    chassis.drive_timeout = 1000;
    chassis.drive_distance(-14);
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(121);//og132
    chassis.drive_timeout = 800;
    chassis.drive_distance(20);
    wait(0.5,sec);

    //go get the 4th ring(on the left) and score onto the goal  
    chassis.drive_distance(-26,121,9,6);
    chassis.turn_timeout = 500;
    chassis.turn_to_angle(153);//100
    chassis.drive_timeout = 800;
    chassis.drive_distance(17);
    chassis.turn_to_angle(118);//130
    chassis.drive_distance(17);
    wait(0.5,sec);
    chassis.drive_distance(-23);
    chassis.turn_timeout = 500;

    //touch the ladder
    chassis.turn_to_angle(43);//210
    intak.stop();
    ringCatcher.spinToPosition(520,degrees,false);
    frontBrake();
    chassis.drive_distance(18,43, 6, 6, 1, 300, 1500);
  } //end of blue side
}// end of four_rings*******************/

void fourRings_elim()
{
  if(current_color_selection==0)//red
  {
    chassis.turn_timeout =300;
    chassis.turn_to_angle(16);
    dunk(); // put preload onto the alliance stake
    ringCatcher.spinToPosition(0,degrees);

    chassis.drive_distance(-30,90,12,9.5, 1,300,1200);
    chassis.drive_distance(-14, 90, 4, 6,1,300,800);
    tilt.set(true);
    wait(0.3,sec);

    chassis.turn_max_voltage = 6;
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(190);
  
    intak.spin(fwd,12,voltageUnits::volt);
    chassis.drive_distance(21,190,12,6,1,300,1000);
    intakeRunning = true;
    wait(0.5,sec);

    //go get the third ring(on the right) and score onto the goal
    chassis.drive_max_voltage = 12;
    chassis.drive_timeout = 1000;
    chassis.drive_distance(-14);
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(239);//og132
    chassis.drive_timeout = 800;
    chassis.drive_distance(20);
    wait(0.5,sec);

    //go get the 4th ring(on the left) and score onto the goal  
    chassis.drive_distance(-26,239,9,6);
    chassis.turn_timeout = 500;
    chassis.turn_to_angle(207);//100
    chassis.drive_timeout = 800;
    chassis.drive_distance(17);
    chassis.turn_to_angle(242);//130
    chassis.drive_distance(16);
    wait(0.5,sec);
    chassis.drive_distance(-5);
    intakeRunning = false;
    intak.stop();
  } // end of red 
  else //blue
  {
    chassis.turn_timeout =300;
    chassis.turn_to_angle(344);
    dunk(); //put the preload onto the alliance stake
    ringCatcher.spinToPosition(0,degrees,false); //reset the ringCatcher

    //go get the goal
    chassis.drive_distance(-30,270,12,8.7,1,300,1600);
    chassis.drive_distance(-14, 270, 4, 6);
    tilt.set(true);
    wait(0.3,sec);

    chassis.turn_max_voltage = 6;
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(170);
  
    intak.spin(fwd,12,voltageUnits::volt);

    chassis.drive_max_voltage = 12;
    chassis.drive_timeout = 1000;
    chassis.drive_distance(21);
    wait(0.5,sec);

    //go get the third ring(on the right) and score onto the goal
    chassis.drive_timeout = 1000;
    chassis.drive_distance(-14);
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(121);//og132
    chassis.drive_timeout = 800;
    chassis.drive_distance(20);
    wait(0.5,sec);

    //go get the 4th ring(on the left) and score onto the goal  
    chassis.drive_distance(-26,121,9,6);
    chassis.turn_timeout = 500;
    chassis.turn_to_angle(153);//100
    chassis.drive_timeout = 800;
    chassis.drive_distance(17);
    chassis.turn_to_angle(118);//130
    chassis.drive_distance(17);
    wait(0.5,sec);
    chassis.drive_distance(-5);
    intak.stop();
  } //end of blue side
}// end of fourRings_elim*******************/

/**********************/
void ringCatcherReset()
{
  loading = 1;
  if (abs(ringCatcher.position(degrees))>20)
  { 
    ringCatcher.setVelocity(80,percent);
    ringCatcher.spinToPosition(20,degrees);
    ringCatcher.setVelocity(20,percent);
    ringCatcher.spinToPosition(2,degrees);
  }
  else 
  {
    ringCatcher.setVelocity(20,percent);
    ringCatcher.spinToPosition(2,degrees);
  }


  loading = 0;
  loadingIntake = 0;
}

void neutralStake()
{
  chassis.drive_distance(44, 0, 10, 6, 1, 300, 1500);
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(33);
  //chassis.drive_distance(2, 35, 4, 6, 1, 300, 500);
  //ringCatcher.setVelocity(20,percent);
  ringCatcher.spin(fwd,6,voltageUnits::volt);
  wait(0.1,sec);
  while(90-ringCatcherRotation.position(degrees)>0.5)
  {
    //if (120-ringCatcherRotation.position(degrees)>20)
   // { ringCatcher.spin(fwd,6,voltageUnits::volt);}
  /*  else 
    {
      if (120-ringCatcherRotation.position(degrees)>0)
      { ringCatcher.spin(fwd,4,voltageUnits::volt);}
            cout<<" position="<<ringCatcherRotation.position(degrees)<<"\n";
    }*/
    
    wait(0.02,sec);
    cout<<" position="<<ringCatcherRotation.position(degrees)<<"\n";
    //if (!ringCatcher.isSpinningMode()) {break;}
  }
  ringCatcher.stop();
  //chassis.drive_distance(-7, 35, 4, 6, 1, 300, 500);
  //ringCatcher.spinFor(reverse,0.3, seconds);
}





/******************/
void dunk_awp_red()
{
  chassis.turn_timeout =300;
  chassis.turn_to_angle(16);
  dunk();
  ringCatcher.spinToPosition(0,degrees);
  //chassis.drive_distance(-5);
  chassis.turn_timeout =500;
  //chassis.turn_to_angle(76);
  chassis.drive_distance(-30,90,12,9.8,1,300,1200);
  chassis.drive_distance(-10.5, 90, 6, 6);
  tilt.set(true);
  wait(0.3,sec);

  chassis.turn_max_voltage = 6;
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(190);
  
  intak.spin(fwd,12,voltageUnits::volt);

  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 1000;
  chassis.drive_distance(21);
  wait(0.5,sec);

  //go get the third ring(on the right) and score onto the goal
  chassis.drive_timeout = 1000;
  chassis.drive_distance(-14);
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(239);//og132
  chassis.drive_timeout = 800;
  chassis.drive_distance(20);
  wait(0.5,sec);

  //go get the 4th ring(on the left) and score onto the goal  
  chassis.drive_distance(-26,239,9,6);
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(207);//100
  chassis.drive_timeout = 800;
  chassis.drive_distance(17);
  chassis.turn_to_angle(242);//130
  chassis.drive_distance(16);
  wait(0.5,sec);
  chassis.drive_distance(-23);
  chassis.turn_timeout = 500;

  //touch the ladder
  chassis.turn_to_angle(317);//210
  intak.stop();
  ringCatcher.spinToPosition(520,degrees,false);
  frontBrake();
  chassis.drive_distance(20,317, 6, 6, 1, 300, 1500);

}

void dunk_awp_blue()
{
  chassis.turn_timeout =300;
  chassis.turn_to_angle(344);
  dunk(); //put the preload onto the alliance stake
  ringCatcher.spinToPosition(0,degrees,false); //reset the ringCatcher

  //go get the goal
  chassis.drive_distance(-30,270,12,8.7,1,300,1600);
  chassis.drive_distance(-14, 270, 4, 6); //10.5
  tilt.set(true);
  wait(0.3,sec);

  chassis.turn_max_voltage = 6;
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(170);
  
  intak.spin(fwd,12,voltageUnits::volt);

  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 1000;
  chassis.drive_distance(21);
  wait(0.5,sec);

  //go get the third ring(on the right) and score onto the goal
  chassis.drive_timeout = 1000;
  chassis.drive_distance(-14);
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(121);//og132
  chassis.drive_timeout = 800;
  chassis.drive_distance(20);
  wait(0.5,sec);

  //go get the 4th ring(on the left) and score onto the goal  
  chassis.drive_distance(-26,121,9,6);
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(153);//100
  chassis.drive_timeout = 800;
  chassis.drive_distance(17);
  chassis.turn_to_angle(118);//130
  chassis.drive_distance(17);
  wait(0.5,sec);
  chassis.drive_distance(-23);
  chassis.turn_timeout = 500;

  //touch the ladder
  chassis.turn_to_angle(43);//210
  intak.stop();
  ringCatcher.spinToPosition(520,degrees,false);
  frontBrake();
  chassis.drive_distance(20,43, 6, 6, 1, 300, 1500);
  
}
/*void ringDetected()
{
  color detectedColor = ringColorSensor.color();
  
  if(current_color_selection==0) //red alliance
  {//if blue ring is detected and in position
    cout<<"found ring\n";
   //if (RingDistance.objectDistance(mm) < 37 && detectedColor == blue)
    if( detectedColor == blue)
    {cout<<"ring** found blue ring\n";
      
      wait(0.1,sec);
      intak.stop(brakeType::hold); 
      //stop intake for 0.4 second so the blue ring will fall out
      wait(0.4,sec);
      intak.spin(fwd,100,velocityUnits::pct); //start the intake again
    //  ringColorSensor.setLight(ledState::off);
    }
  }
  else if (current_color_selection==1) //blue alliance
  {
    //if red ring is detected and in position
    if(RingDistance.objectDistance(mm) < 37 && detectedColor == red)
    {
      intak.stop(brakeType::hold); 
      //stop intake for 0.4 second so the blue ring will fall out
      wait(0.4,sec);
      intak.spin(fwd,100,velocityUnits::pct); //start the intake again
     // ringColorSensor.setLight(ledState::off);
    }
  }
}*/// end of ringDectected()


/**********auton skills program***********/
void skills()
{
  //distance between clamp and wall stake is 3/8 spacer
  //BETTER TO BE CLOSER THAN FARTHER FROM THE WALL STAKE
  //ring/intake tooth at the top of the intake flex wheels
 // ringColorSensor.objectDetected(ringDetected);  

  //spin preload onto alliance stake
  intak.spin(fwd,12,voltageUnits::volt);
  wait(0.5,sec);
  intak.stop();

  //get mobile goal
  chassis.drive_distance(19, 0, 12, 6, 1, 300, 2000);//22
  chassis.turn_timeout = 700;
  chassis.turn_to_angle(270);
  chassis.drive_distance(-10, 270, 12, 6, 1, 300, 500);
  chassis.drive_distance(-10, 270, 6, 6, 1, 300, 800);//-16.5
  tilt.set(true);
  wait(0.3,sec);
  chassis.drive_distance(-6, 270, 12, 6, 1, 300, 500);
  //get the other rings, starting with top right ring
  //go to grab first ring
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.turn_to_angle(0);
  intakeRunning = true;
  chassis.drive_distance(23, 0, 8, 6, 1, 300, 1500);
  wait(0.5,sec);
  chassis.turn_timeout = 800;
 
  //second ring
  chassis.turn_to_angle(90);
  chassis.drive_distance(30);
  chassis.drive_distance(-8);

  //next two rings(3d, 4th)
  chassis.turn_to_angle(180);
  chassis.drive_max_voltage = 8;
  chassis.drive_timeout = 800;
  chassis.drive_distance(23);
  wait(0.5,sec);
  chassis.drive_timeout = 600;
  chassis.drive_distance(12); 
  wait(0.5,sec);//

  //last ring
  chassis.turn_timeout = 1000;
  chassis.turn_to_angle(42);
  chassis.drive_timeout = 1000;
  chassis.drive_distance(15);
  wait(1,sec);

  //put goal in the corner
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(342);
  chassis.drive_timeout = 1000;
  chassis.drive_distance(-12);//15
  intakeRunning=false;
  intak.stop();
  tilt.set(false);
  chassis.drive_distance(10,330,12,6,1,300,400); //move out of the corner
  chassis.turn_timeout = 1000;
  chassis.turn_to_angle(90);
/*
  //get the goal on the left side
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 7000;
  chassis.drive_distance(-70, 90, 12, 6, 1, 300, 3000);
  chassis.drive_max_voltage = 6;
  chassis.drive_timeout = 1500;
  chassis.drive_distance(-21, 90, 6, 6, 1, 300, 1200); 
  tilt.set(true);
  wait(0.3,sec);
 
  //intake the 1st ring
  chassis.drive_max_voltage = 8;
  chassis.turn_timeout = 1000;
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.turn_to_angle(0);
  intakeRunning=true;
  chassis.drive_distance(26.5);
  wait(0.5,sec);
  chassis.turn_to_angle(270);
  
  //intake the 2nd ring
  chassis.drive_distance(26);
  wait(0.5,sec);
  chassis.drive_distance(-2);
  chassis.turn_to_angle(180);
  //intake the next 2 rings
  chassis.drive_distance(21);
  wait(0.5,sec);
  chassis.drive_distance(12);
  //last ring
  intakeRunning=false;
  intak.stop();
  chassis.turn_to_angle(312);//317
  intakeRunning=true;
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_timeout = 500;
  chassis.drive_distance(14);
  wait(0.8,sec);

  //put goal in the corner
  chassis.turn_timeout = 800;
  intakeRunning=false;
  intak.stop();
  chassis.turn_to_angle(22);
  intakeRunning=true;
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_distance(-14, 22, 12, 6, 1, 300, 800);//-11
  wait(0.1,sec);
  tilt.set(false);
  wait(0.3,sec);
  intakeRunning=false;
  intak.stop();
  wait(0.3,sec);

  chassis.drive_distance(33, 22, 12, 6, 1, 300, 1500);//36
  chassis.turn_timeout = 1000;
  chassis.drive_timeout = 2000;
  
  //start to go to the other side
  chassis.turn_to_angle(0);
  //intake first ring
  intakeRunning=true;
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_distance(62, 0, 12, 6, 1, 300, 1500);
  intakeRunning = false;
  intak.stop();
  //chassis.drive_distance(-7, 0, 12, 6, 1, 300, 800);//8
  //get the mobile goal with blue ring on it
  chassis.turn_to_angle(215);//215
  chassis.drive_distance(-27, 215, 12, 6, 1, 300, 750);
  chassis.drive_distance(-11.5, 215, 6, 6, 1, 300, 650); //
  tilt.set(true);
  wait(0.5,sec);

  chassis.turn_timeout = 700;
  chassis.turn_to_angle(270);
  intak.spin(fwd,12,voltageUnits::volt);
  intakeRunning = true;
  chassis.drive_distance(30, 270, 8, 6, 1, 300, 1300);
  chassis.turn_timeout = 660;
  chassis.turn_to_angle(60);
  chassis.turn_to_angle(132);//127
  tilt.set(false); //release goal
  chassis.drive_distance(-6, 132, 12, 6, 1, 300, 500);

  //get the 2nd mobile goal 
  //chassis.drive_distance(21, 125, 12, 6, 1, 300, 1100);
  chassis.drive_distance(19, 125, 12, 6, 1, 300, 1100);//21
  chassis.turn_timeout = 1200;
  chassis.turn_to_angle(270);
  chassis.drive_distance(-26, 270, 12, 6, 1, 300, 1300);
  chassis.drive_distance(-19, 270, 6, 6, 1, 300, 1000); //og 11
  intakeRunning = false;
  intak.stop();  
  tilt.set(true);
  wait(0.3,sec);

  //get the next rings
  intak.spin(fwd,12,voltageUnits::volt);

  chassis.turn_to_angle(130);
  intakeRunning = true;
  chassis.drive_distance(39);
  chassis.turn_to_angle(90);
  chassis.drive_distance(32);

  chassis.turn_timeout = 800;
  chassis.turn_to_angle(15);//30
  //put the last mobile goal in the corner
  chassis.drive_timeout = 1000;
  chassis.drive_distance(30);//35
  chassis.turn_timeout = 1500;
  chassis.turn_to_angle(220);
  intakeRunning = false;
  intak.stop();
  chassis.drive_distance(-5,220,12,6,1,300,380);
  tilt.set(false);
  //re-push
  chassis.drive_distance(5,220,12,6,1,300,380);
  chassis.drive_distance(-7,220,12,6,1,300,380);
  chassis.drive_distance(10);
*/  
}//end of skills()

void driveTest()
{
  chassis.drive_distance(24,0,12,6,1,300,2000);
}
void autonomous(void) {
  auto_started = true;
  if (auto_started)
  {
    Brain.Screen.printAt(50, 50, "AWP solo");
    skills(); //slot 1
    //fourRings_AWP();//slot 2
    //middlegoal_qual(); //slot 3
    //fourRings_elim();//slot 4
    //driveTest();
    //dunk_awp_blue(); //slot5
    //dunk_awp_red(); //slot6
  }
}// end of autonomous()

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              User Control Task                            */
/*                                                                           */
/*  This task is used to control your robot during the user control phase of */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

bool til = false;
// goal clamp
void toggleTilt(){
  if(!til){
    tilt.set(true);
    til = true;
  }
  else{
    tilt.set(false);
    til = false;
  }
}

/********************************************************/
//use distance sensor to load the ring box automaticlly
//when the distance is <40mm, intake stops for 0.5sec
//then reverse intake to put the ring into the ringbox

/*void loadBox()
{
  loading =1; //start loading the ring box

  intak.spin(fwd,60,velocityUnits::pct);

  //when the distance sensor sees an object <40mm
  while ((RingDistance.objectDistance(mm) > 50)) //40
  {
    wait(5, msec);
  }
  intak.stop(); //stop intake
  wait(0.3,sec);
  intak.spin(reverse,100,velocityUnits::pct);//spin intake reversely
  wait(0.5,sec);
  intak.stop();
  loading =0; //end loading the ring box
}*///loadBox

//bool intakeRunning=false;
void intakeJam()
{
   // cout<<"check intake"<<endl;
 if (intakeRunning) {
  // cout<<"intake is running"<<endl;
    if (fabs(static_cast<float>(intak.velocity(percent))) < 5.0) {
   //   cout<<"intake jam!"<<endl;
      foundIntakeJam = true;
      intak.spin(reverse, 12,voltageUnits::volt);
      wait(0.2, seconds);
      intak.spin(fwd, 12 ,voltageUnits::volt);
      wait(0.5, sec);
      foundIntakeJam = false;
    }
  }
}
/*********toggle ringColorSorting options*********/
/*void toggle_colorSorting()
{
  if (ringColorSorting) {ringColorSorting = false;}
  else { ringColorSorting = true; }
}*/// end of toggle_colorSorting()

/*********toggle the fan options*********/
void toggle_fan()
{
  if (fanOut) {doink.set(false); fanOut = false;}
  else { doink.set(true); fanOut = true;}
}// end of toggle_fan()



void ringCatcherLoad()  
{
   loading = 1;
/*   cout<<"before loading ring catcher position="<<ringCatcherRotation.position(degrees);
  while(abs(ringCatcherRotation.position(degrees)-17.5)>0.3)
  {
    if (ringCatcherRotation.position(degrees)<17.5)
    { ringCatcher.spin(fwd,3,voltageUnits::volt);}
    else {ringCatcher.spin(reverse,3,voltageUnits::volt);}
        wait(0.02,sec);
  }
  ringCatcher.stop();
 // ringCatcher.setVelocity(50,percent);
 
 // ringCatcher.spin(fwd);
 //  wait(1,sec);
//  ringCatcher.spinToPosition(115,degrees);
   cout<<" after loading ring catcher position="<<ringCatcherRotation.position(degrees)<<"\n";*/

  double target =18;//og 17.5
  double current =0.0;
  double error =target-current;
  double preverror = error;
  double p=0.2,d=0.1;
  double delerror =preverror-error;
  int count=0;
  while(abs(error)>=0.2){
    current = abs(ringCatcherRotation.position(degrees));
    cout<<"curr: "<<current<<endl;
    error = target - current;
    cout<<"error: "<<error<<endl;
    delerror =preverror - error;;
    float nextvolt = abs(error) * p + delerror * d;
    cout<<"nextvolt="<<nextvolt<<endl;
    if(nextvolt>12) {nextvolt = 12;}
    if(nextvolt<2) {nextvolt = 2;}
    //nextvolt = max(nextvolt,2);
    //nextvolt = min(nextvolt,12);
    
    if (error>0) { ringCatcher.spin(fwd,nextvolt,voltageUnits::volt); }
    else { ringCatcher.spin(reverse,nextvolt,voltageUnits::volt); }
    preverror = error;
    wait(0.01,sec);
    count++;
  }
  ringCatcher.stop();
  current = ringCatcherRotation.position(degrees);
    cout<<"end: "<<current<<" count = "<<count<<endl;
   loading = 0;
   loadingIntake = 1;
}

void sortingRingColors()
{
  if (current_color_selection == 0) //red alliance, sort out blue ring
  { 
    if (ringColorSensor.isNearObject() && ringColorSensor.color() == blue) {
      ringColorCorrect = false;
      Brain.Screen.print("Blue ring Found");
      Brain.Screen.newLine();
      wait(0.12,sec);
      intak.stop();
      wait(0.1,sec);
      ringColorCorrect = true;
     // intak.spin(fwd);
    }
  }// end of red
  else if (current_color_selection == 1) //blue alliance, sort out ring ring
  {
    if (ringColorSensor.isNearObject() && ringColorSensor.color() == red) {
      Brain.Screen.print("Blue ring Found");
      intak.stop();
      wait(0.2,sec);
      intak.spin(fwd);
    }
  }// end of red
}

void usercontrol(void) 
{
  ringColorSensor.setLight(ledState::on); //turn on the optical sensor light
  ringColorSensor.setLightPower(100, percent);
  // User control code here, inside the loop
  while (1) {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.
    if(Controller1.ButtonR1.pressing())
    {
      if (loadingIntake ) {intak.spin(fwd,9, voltageUnits::volt);}
      else if(ringColorCorrect)
      { intak.spin(fwd,12, voltageUnits::volt); }
    }
    else if(Controller1.ButtonR2.pressing()){
      if (loadingIntake) {intak.spin(reverse,9, voltageUnits::volt);}
      else { intak.spin(reverse,12, voltageUnits::volt); }
    }
    else if(!loading)
    {
      intak.stop();
    }

    if(Controller1.ButtonL1.pressing()){  // ring catcher going up
      ringCatcher.stop();
      if (abs(ringCatcherRotation.position(degrees))<160)
      {//cout<<"here1"<<endl;
        ringCatcher.spin(fwd,4,voltageUnits::volt);}
      else {ringCatcher.stop();}
    } //L1 pressing
    else if(Controller1.ButtonL2.pressing())
    {  //ring catcher going down
      ringCatcher.stop();
      if (abs(ringCatcherRotation.position(degrees))>2)
      {ringCatcher.spin(reverse,4,voltageUnits::volt);}
      else {ringCatcher.stop();}
    }// L2 pressing
    else if(!loading && !dunking)
    {
      ringCatcher.stop();
    } //end of if(!Controller1.ButtonR1.pressing())
    //cout<<"ring catcher position="<<ringCatcher.position(degrees)<<" ring catcher rotation="<<ringCatcherRotation.position(degrees)<<"\n";
    
    
    int a3 = Controller1.Axis3.position();
    int a2 = Controller1.Axis2.position();
    if (a2>10) {cout<<"a2="<<a2<<" a3="<<a3<<"\n";}
    if (abs(a3)>  10|| abs(a2)> 10) //to avoid joystick drift and random touch
    {
      int leftSpeed = (a3-10)*10/9;
      int rightSpeed = (a2-10)*10/9;
      runChassis(leftSpeed,rightSpeed);
    }
    else
    {
      Chicken.stop(brakeType::coast);
    }
    /*if(Controller1.ButtonRight.pressing() && Controller1.ButtonY.pressing())
    {
      elevation.set(true);
    }*/
    wait(20, msec); // Sleep the task for a short amount of time to
                    // prevent wasted resources.
  }
}

//
// Main will set up the competition functions and callbacks.
//
int main() {
  // Run the pre-autonomous function.
  pre_auton();
  
 // cout<<"the code is running\n";
  ringColorSensor.setLightPower(100, percent);
  ringColorSensor.setLight(ledState::on);
  
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);
  checkIntake(intakeJam);
  ringColorCheck(sortingRingColors);
  Controller1.ButtonA.pressed(toggleTilt);
  Controller1.ButtonY.pressed(printTemps);
  Controller1.ButtonUp.pressed(dunk);
  Controller1.ButtonDown.pressed(ringCatcherReset);
  Controller1.ButtonRight.pressed(ringCatcherLoad);
  //Controller1.ButtonUp.pressed(toggle_colorSorting);// button up pressed to turn on/off color sorting
  Controller1.ButtonB.pressed(toggle_fan);// 
  
  
  // Prevent main from exiting with an infinite loop.
  while (true) {
    if (!foundIntakeJam) 
    {
      checkIntake.broadcast();
    }
    if (ringColorSorting) 
    {  
      ringColorCheck.broadcast();  
    }
    wait(100, msec);
  }
}