#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// tr                   motor         11              
// fr                   motor         1               
// bl                   motor         10              
// fl                   motor         19              
// br                   motor         12              
// tl                   motor         9               
// intak                motor         2               
// tilt                 digital_out   B               
// elevation            digital_out   F               
// doink                digital_out   C               
// ringCatcher          motor         14              
// ringCatcherRotation  rotation      16              
// ringColorSensor      optical       21              
// ringStopper          digital_out   D               
// ---- END VEXCODE CONFIGURED DEVICES ----


using namespace vex;

// A global instance of competition
competition Competition;

inertial LBJInertial = inertial(PORT6);
vex::distance ringDistance = vex::distance(PORT8); //somehow, adding this to device list makes error

motor_group leftDrive (fl, tl, bl);
motor_group rightDrive (fr, tr, br);
smartdrive Chicken(leftDrive, rightDrive, LBJInertial, 10.205,10.75,11,distanceUnits::in);

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
PORT6,

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

int current_color_selection = 0; //0: red alliance; 1: blue alliance; 
bool doAllianceStake = false;// true: score alliance stake during auton; false: NOT score alliance stake during auton

//bool auto_started = false; //for competition matches
bool auto_started = true; //for skills and practice

//bool ringColorSorting = false; 
bool ringColorSorting = true; //use optical sensor to sort ring colors
bool ringColorCorrect = true; //if the ring color is the same with the alliance color;
bool fanOut = false; //if the doinker is out or not
bool ringStopperOut = false; //if the LB ring stopper is out or not
bool loadingIntake = 0; //if the robot is in the status of loading LB
bool loading = 0; //not load the ring box
bool dunking = 0; //if the LB is in  the dunking process or not
bool inLoadingPosition = 0; //if LB is in the loading position
bool loadingRingInProgress = 0; //if LB is in the loading process
float intakeVoltage_scoringGoals = 10.0; //voltage to run intake for scoring the MOGO
float intakeVoltage_loadingLB = 10.0;//voltage to run intake for loading LB
bool intakeRunning=false; //if the intake is running
bool foundIntakeJam = false; //if intake jam is found

event checkIntake = event(); //check intake to find intake jam
event ringColorCheck = event(); //check ring color to find rings with wrong color
event LBLoadingCheck = event(); //check LB to see it is loaded

/**********************************************************************/
//print each motor temperature on the brain
void printTemps() 
{     
      Brain.Screen.clearScreen();
      Brain.Screen.print(" bl T=");
      Brain.Screen.print(fl.temperature(celsius));
      Brain.Screen.print(" fl T=");
      Brain.Screen.print(fl.temperature(celsius));
      Brain.Screen.print(" tl T=");
      Brain.Screen.print(fl.temperature(celsius));

      Brain.Screen.newLine();
      Brain.Screen.print(" br T=");
      Brain.Screen.print(br.temperature(celsius));
      Brain.Screen.print("fr T=");
      Brain.Screen.print(fr.temperature(celsius));
      Brain.Screen.print(" tr T=");
      Brain.Screen.print(fl.temperature(celsius));
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

/**********************************************************************/
//set different brake types for the drive train
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

void coastBrake()
{
  bl.stop(coast);
  br.stop(coast);
  tl.stop(coast);
  tr.stop(coast);
  fl.stop(coast);
  fr.stop(coast);
}//end of stopBrake()

void brakeBrake()
{
  bl.stop(brake);
  br.stop(brake);
  tl.stop(brake);
  tr.stop(brake);
  fl.stop(brake);
  fr.stop(brake);
}//end of stopBrake()

/**********************************************************************/
void pre_auton(void) 
{
  Brain.Screen.setFont(vex::fontType::mono60); 
  vexcodeInit();
  LBJInertial.calibrate(); //calibrate inertial sensor
  default_constants();
  
  //set pneumatics starting status
  tilt.set(false); //clamp up
  doink.set(false); //doinker up
  ringStopper.set(false); //ring stopper off

  //set motors and sensors starting status
  ringCatcher.setPosition(0, degrees);
  ringCatcher.setVelocity(100,percent);
  ringCatcher.setStopping(hold);
  ringCatcherRotation.setPosition(0, degrees);
  ringCatcherRotation.setReversed(true);
  
  int autonChoice = 0; 
  while(auto_started == false){            
    Brain.Screen.clearScreen();
    switch(autonChoice){       
      case 0:
        Brain.Screen.setPenColor(red);
        Brain.Screen.printAt(30, 100, "Red, Alli STK");
        break;
      case 1:
        Brain.Screen.setPenColor(red);
        Brain.Screen.printAt(30, 100, "Red , NO Alli STK");
        break;
      case 2:
        Brain.Screen.setPenColor(blue);
        Brain.Screen.printAt(30, 100, "Blue, Alli STK");
        break;
      case 3:
        Brain.Screen.setPenColor(blue);
        Brain.Screen.printAt(30, 100, "Blue, NO Alli STK");
        break;
    }

    //before auton starts, after select the program,
    //tap the screen to loop through the 4 auton choices
    if(Brain.Screen.pressing()){
      while(Brain.Screen.pressing()) {}
      autonChoice ++;
    } else if (autonChoice == 4){
      autonChoice = 0;
    }
    if (autonChoice == 0) 
    {
      current_color_selection = 0; //red
      doAllianceStake = 1; //do alliance stake
    }
    else if (autonChoice == 1) 
    {
      current_color_selection = 0; //red
      doAllianceStake = 0; //NOT do alliance stake
    }
    else if (autonChoice == 2)
    {
      current_color_selection = 1; // blue
      doAllianceStake = 1; //do alliance stake
    }
    else if (autonChoice == 3)
    {
      current_color_selection = 1; //blue
      doAllianceStake = 0;  //do NOT do alliance stake
    }
    else 
    {
      Brain.Screen.setPenColor(red);
      Brain.Screen.printAt(100, 100, "ERROR!");
    }
    task::sleep(10);
  } //while
} //end of pre_auton

/**********************************************************************/
//this function returns the LB to the reset position
//it reads the current position of the ring catcher rotation sensor,
//compares with the target position. If the difference between the target and current
//is less than 0.5 degree, stop the LB motor;
//if the difference is > 0.5 degree, move LB toward the target
//use PID to control the velocity of the arm
void ringCatcherReset()
{
  ringStopper.set(false);// retract the ring stopper
  ringStopperOut = 0;
  loading = 1; 
  double target = 5; //target position
  double current = 0.0; //current position
  double error =target-current; //difference between the target and current position
  double preverror = error; //previous error, for Kp 
  double p=0.3,d=0.5;
  double delerror =preverror-error; //delta error = previous error - current error
  int count=0;
  while(fabs(error)>=0.5){ //if |target-current|>=0.5 degree
    current = ringCatcherRotation.position(degrees);
    error = target - current;
    delerror =preverror - error;;
    float nextvolt = fabs(error) * p + delerror * d; //calculate the voltage to run LB
    if(nextvolt>12) {nextvolt = 8;} //limit the LB motor voltage to 8V so it doesn't move too fast
    else if(nextvolt<2) {nextvolt = 2;} 
    else { nextvolt =  nextvolt/12*8;}
    
    if (error>0) { ringCatcher.spin(fwd,nextvolt,voltageUnits::volt); }
    else { ringCatcher.spin(reverse,nextvolt,voltageUnits::volt); }
    preverror = error;
    wait(0.01,sec);
    count++;
  } //end of while(fabs(error)>=0.5)
  dunking = 0;
  ringCatcher.stop();
  loading = 0;
  loadingIntake = 0;
}//end of ringCatcherReset
/**********************************************************************/

/**********************************************************************/
//this function moves the LB to the scoring position
//it reads the current position of the ring catcher rotation sensor,
//compares with the target position. If the difference between the target and current
//is less than 0.5 degree, stop the LB motor;
//if the difference is > 0.5 degree, move LB toward the target
//use PID to control the velocity of the arm
void dunk()
{
  dunking = 1;
  double target = 140;
  double current =0.0;
  double error =target-current;
  double preverror = error;
  double p=0.3,d=0.5;
  double delerror =preverror-error;
  int count=0;
  while(fabs(error)>=1 && !loading){
    current = ringCatcherRotation.position(degrees);
    cout<<"curr: "<<current<<endl;
    error = target - current;
    cout<<"error: "<<error<<endl;
    delerror =preverror - error;;
    float nextvolt = fabs(error) * p + delerror * d;
 
    if(nextvolt>12) {nextvolt = 12;}
  //  else {nextvolt = nextvolt/12*10; }
    if(nextvolt<2) {nextvolt = 2;}
    if (error>0) { ringCatcher.spin(fwd,nextvolt,voltageUnits::volt); }
    else { ringCatcher.spin(reverse,nextvolt,voltageUnits::volt); }
    preverror = error;
    wait(0.01,sec);
    count++;
  } //end of while(fabs(error)>=1 && !loading)
  dunking = 0;
  ringCatcher.stop();
} // end of dunk()

/**********************************************************************/
void ringCatcherLoad()  
{
  ringStopper.set(false);
  ringStopperOut = 0;
  loading = 1;


  double target = 21; // catch the first ring
  //double target =125; //catch the second ring
  double current =0.0;
  double error =target-current;
  double preverror = error;
  double p=0.3,d=0.1;
  double delerror =preverror-error;
  int count=0;
  while(fabs(error)>0.5){
    current = fabs(ringCatcherRotation.position(degrees));
    cout<<"curr: "<<current<<endl;
    error = target - current;
    cout<<"error: "<<error<<endl;
    delerror =preverror - error;;
    float nextvolt = fabs(error) * p + delerror * d;
    cout<<"nextvolt="<<nextvolt<<endl;
    if(nextvolt>8) {nextvolt = 8;}
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
   inLoadingPosition =1;
   loadingIntake = 1;
}

/**********************************
This auton is for qualification match, 
when our robot starts on the positive corner side;
user chooses from the brain :
1. red or blue alliance
2. whether or not to score on the alliance stake
***********************************/
void posSide_qual()
{
  if(current_color_selection==0 && !doAllianceStake)
  //red, NOT score on alliance stake
  //Robot starts in front of the goal, back facing the goal
  //preload ring on the intake
  /************************************************** 
  |                                   G2   |
  |                                        |
  |                             G1    R1   |
  |                                        |
  |                    R      Robot   R    |
  |RN                  R                 RP|
   -------------------STK----------------------
                 red alliance
  **************************************************/  
  {
    //Step 1: go north to get G1 and score preload into G1
    chassis.drive_distance(-16, 0, 6, 6, 1, 300, 800);
    chassis.drive_distance(-13, 0, 4, 6, 1, 300, 800);
    tilt.set(true); //clamp on the goal
    wait(0.3,sec); 
    //score the preload onto the goal
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.turn_to_angle(260,6,1,300,800);
    intakeRunning = true;
    tilt.set(false); //release G1

    //Step 2: go east to get R1 and store in intake
    intak.spin(fwd,9,voltageUnits::volt);
    chassis.drive_distance(27,260,12,6,1,300,1000);
    wait(0.1,sec);
    intakeRunning = false;
    intak.stop();
    chassis.drive_distance(-3,260,12,6,1,300,300);

    //Step 3: go north to get G2, score R1 into G2
    chassis.turn_to_angle(0,4,1,300,1000);
    chassis.drive_distance(-14,0,6,6,1,300,600);
    chassis.drive_distance(-5, 0, 4, 6, 1, 300, 400);
    tilt.set(true);
    wait(0.3,sec);
    //put R1 on G2
    intak.spin(fwd,10,voltageUnits::volt);

    //Step 4: go touch the ladder for potential AWP
    chassis.drive_distance(30,30,12,10, 1,300,1500);
    chassis.turn_to_angle(135,6,1,300,1000);
    intakeRunning = false;
    intak.stop();
    //raise lady brown to touch the bar
    ringCatcher.spinToPosition(800,degrees,false);
    chassis.drive_distance(13, 135, 8, 6, 1, 300, 1500);
  } // end of red alliance, NOT score on alliance stake
  else if(current_color_selection==1 && !doAllianceStake)
  //blue alliance, NOT score on alliance stake
  //Robot starts in front of the goal, back facing the goal
  //preload ring on the intake
  /************************************************** 
  |   G2                                   |
  |                                        |
  |   R1    G1                             |
  |                                        |
  |   R    Robot       R                   |
  |BP                  R                 BN|
   -------------------STK----------------------
                 blue alliance
  **************************************************/ 
  {
    //Step 1: go north to get G1 and score preload into G1
    chassis.drive_distance(-16, 0, 6, 6, 1, 300, 800);
    chassis.drive_distance(-13, 0, 4, 6, 1, 300, 800);
    tilt.set(true); //clamp on the goal
    wait(0.3,sec); 
    //score the preload onto G1
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.turn_to_angle(100,6,1,300,800);
    intakeRunning = true;
    tilt.set(false); //release G1

    //Step 2: go east to get R1 and store in intake
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.drive_distance(27,100,12,6,1,300,1000);
  //  wait(0.1,sec);
    intakeRunning = false;
    intak.stop();
    chassis.drive_distance(-3,100,12,6,1,300,300);

    //Step 3: go north to get G2, score R1 into G2
    chassis.turn_to_angle(0,4,1,300,1000);
    chassis.drive_distance(-14,0,6,6,1,300,600);
    chassis.drive_distance(-5, 0, 4, 6, 1, 300, 400);
    tilt.set(true);
    wait(0.3,sec);
    //put R1 on G2
    intak.spin(fwd,10,voltageUnits::volt);

    //Step 4: go touch the ladder for potential AWP
    chassis.drive_distance(30,330,12,10, 1,300,1500);
    chassis.turn_to_angle(225,6,1,300,1000);
    intakeRunning = false;
    intak.stop();
    //raise lady brown to touch the bar
    ringCatcher.spinToPosition(800,degrees,false);
    chassis.drive_distance(16, 225, 8, 6, 1, 300, 1500);
  } //end of blue alliance, not score on alliance stake
  else if(current_color_selection==0 && doAllianceStake)
  //red alliance, score on the alliance stake
  //Robot starts facing the rings in front of the alliance stake
  //preload ring on LB arm
  /************************************************** 
  |                                   G2   |
  |                                        |
  |                             G1    R1   |
  |                                        |
  |                    R              R    |
  |RN                  R Robot           BP|
   -------------------STK----------------------
                 red alliance
  **************************************************/ 
  {
    ringCatcher.setPosition(0,degrees);
    chassis.turn_to_angle(340, 6, 0.5, 300, 300); //turn to face the alliace stake
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);
    
    chassis.drive_distance(-30,270,12, 8.5, 1,300,1100);
    chassis.drive_distance(-12, 270, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.3,sec);

    //Step 2: go east to get R1 and score into G1
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.turn_to_angle(180,6,1,300,800);
    intakeRunning = true;
    chassis.drive_distance(27,180,12,6,1,300,1000);
    wait(0.1,sec);

    chassis.turn_to_angle(45,8,1,300,900);
    intakeRunning = false;
    intak.stop();
    tilt.set(false); //drop G1

   //Step 3: go north to get G2
    chassis.turn_to_angle(270,4,1,300,1000);
    chassis.drive_distance(-19,270,6,6,1,300,900);
//    chassis.drive_distance(-3,270,12,6,1,300,300);
    tilt.set(true);
    wait(0.3,sec);

    //Step 4: go touch the ladder for potential AWP
    chassis.drive_distance(30,330,12,10, 1,300,1500);
    chassis.turn_to_angle(45,6,1,300,1000);
    //raise lady brown to touch the bar
    ringCatcher.spinToPosition(700,degrees,false);
    chassis.drive_distance(7, 45, 8, 6, 1, 300, 1500);
  }//end of red alliance, score on alliance stake
  else if(current_color_selection==1 && doAllianceStake)
  //blue alliance, score on alliance stake
  //Robot starts facing the rings in front of the alliance stake
  //preload ring on LB arm
  /************************************************** 
  |   G2                                   |
  |                                        |
  |   R1    G1                             |
  |                                        |
  |   R                R                   |
  |BP           Robot  R                 BN|
   -------------------STK----------------------
                 blue alliance
  **************************************************/ 
  {
    ringCatcher.setPosition(0,degrees);
    chassis.turn_to_angle(19, 6, 0.5, 300, 300); //turn to face the alliace stake
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);//return LB arm to starting position
    
    //go get G1
    chassis.drive_distance(-30,90,12, 9, 1,300,1100);
    chassis.drive_distance(-12, 90, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.3,sec);

    //Step 2: go east to get R1 and score onto G1
    chassis.turn_to_angle(180, 6, 0.5, 300, 800);
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.drive_distance(27,180,12,6,1,300,1000);
    wait(0.1,sec);

    chassis.turn_to_angle(315,8,1,300,900);
    intakeRunning = false;
    intak.stop();
    tilt.set(false); //drop G1

    //Step 3: go north to get G2, score R1 into G2
    chassis.turn_to_angle(88,4,1,500,2000);
    chassis.drive_distance(-19,88,6,6,1,300,600);
  //  chassis.drive_distance(-5, 88, 4, 6, 1, 300, 400);
    tilt.set(true);
    wait(0.3,sec);

    chassis.drive_distance(18,45,12,10, 1,300,1000);
    //raise lady brown to touch the bar
    ringCatcher.spinToPosition(700,degrees,false);
    chassis.drive_distance(24,310,8,6, 1,300,1500);
  }//end of blue alliance, score on alliance stake
}//end of posSide_qual
/********************************************************/

/**********************************
This auton is for elimination match, 
when our robot starts on the positive corner side;
user chooses from the brain :
1. red or blue alliance
2. whether or not to score on the alliance stake
***********************************/
void posSide_elim()
{
  if(current_color_selection==0 && !doAllianceStake)
  //red, NOT score on alliance stake
  //Robot starts in front of the goal G1, back facing G1
  //preload ring on the intake
  /************************************************** 
  |                                   G2   |
  |                                        |
  |                             G1    R1   |
  |                                        |
  |                    R      Robot   R    |
  |RN                  R                 RP|
   -------------------STK------------------
                 red alliance
  **************************************************/  
  {
    //Step 1: go north to get G1 and score preload into G1
    chassis.drive_distance(-16, 0, 6, 6, 1, 300, 800);
    chassis.drive_distance(-13, 0, 4, 6, 1, 300, 800);
    tilt.set(true); //clamp on the goal
    wait(0.3,sec); 
    //score the preload onto the goal
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.turn_to_angle(260,6,1,300,800);
    intakeRunning = true;
    tilt.set(false); //release G1

    //Step 2: go east to get R1 and store in intake
    intak.spin(fwd,9,voltageUnits::volt);
    chassis.drive_distance(27,260,12,6,1,300,1000);
    wait(0.1,sec);
    intakeRunning = false;
    intak.stop();
    chassis.drive_distance(-3,260,12,6,1,300,300);

    //Step 3: go north to get G2, score R1 into G2
    chassis.turn_to_angle(0,4,1,300,1000);
    chassis.drive_distance(-14,0,6,6,1,300,600);
    chassis.drive_distance(-5, 0, 4, 6, 1, 300, 400);
    tilt.set(true);
    wait(0.3,sec);
    //put R1 on G2
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);

    chassis.drive_distance(40,25,12,10, 1,300,1000);
    chassis.drive_distance(30,282,12,10, 1,300,1500);
    intakeRunning = false;
    intak.stop();
    doink.set(true);
    chassis.drive_distance(10,282,12,10, 1,300,1500);
    chassis.turn_to_angle(200, 12, 0.5, 300, 800);
    doink.set(false);
  /*  chassis.turn_to_angle(288, 6, 0.5, 300, 1000);
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.drive_distance(10,288,12,10, 1,300,2000);
    intak.stop();*/
  } // end of red alliance, NOT score on alliance stake
  else if(current_color_selection==1 && !doAllianceStake)
  //blue alliance, NOT score on alliance stake
  //Robot starts in front of the goal, back facing the goal
  //preload ring on the intake
  /************************************************** 
  |   G2                                   |
  |                                        |
  |   R1    G1                             |
  |                                        |
  |   R    Robot       R                   |
  |BP                  R                 BN|
   -------------------STK----------------------
                 blue alliance
  **************************************************/ 
  {
    //Step 1: go north to get G1 and score preload into G1
    chassis.drive_distance(-16, 0, 6, 6, 1, 300, 800);
    chassis.drive_distance(-13, 0, 4, 6, 1, 300, 800);
    tilt.set(true); //clamp on the goal
    wait(0.3,sec); 
    //score the preload onto G1
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.turn_to_angle(100,6,1,300,800);
    intakeRunning = true;
    tilt.set(false); //release G1

    //Step 2: go east to get R1 and store in intake
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.drive_distance(27,100,12,6,1,300,1000);
  //  wait(0.1,sec);
    intakeRunning = false;
    intak.stop();
    chassis.drive_distance(-3,100,12,6,1,300,300);

    //Step 3: go north to get G2, score R1 into G2
    chassis.turn_to_angle(0,4,1,300,1000);
    chassis.drive_distance(-14,0,6,6,1,300,600);
    chassis.drive_distance(-5, 0, 4, 6, 1, 300, 400);
    tilt.set(true);
    wait(0.3,sec);
    //put R1 on G2
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);

    chassis.drive_distance(20,0,8,6, 1,300,1500);
    intakeRunning = false;
    intak.stop();
    chassis.drive_distance(30,20,8,6, 1,300,1500);
    doink.set(true);
    chassis.drive_distance(5,20,12,10, 1,300,500);
    chassis.turn_to_angle(290, 12, 0.5, 300, 800);
    doink.set(false);
/*    chassis.turn_to_angle(25, 6, 0.5, 300, 1000);
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.drive_distance(10,25,12,10, 1,300,2000);
    intak.stop();*/
  } //end of blue alliance, not score on alliance stake
  else if(current_color_selection==0 && doAllianceStake)
  //red alliance, score on the alliance stake
  //Robot starts facing the rings in front of the alliance stake
  //preload ring on LB arm
  /************************************************** 
  |                                   G2   |
  |                                        |
  |                             G1    R1   |
  |                                        |
  |                    R              R    |
  |RN                  R Robot           RP|
   -------------------STK----------------------
                 red alliance
  **************************************************/ 
  {
    ringCatcher.setPosition(0,degrees);
    chassis.turn_to_angle(340, 6, 0.5, 300, 300); //turn to face the alliace stake
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);
    
    chassis.drive_distance(-30,270,12, 8.5, 1,300,1100);
    chassis.drive_distance(-12, 270, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.3,sec);

    //Step 2: go east to get R1 and score into G1
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.turn_to_angle(180,6,1,300,800);
    intakeRunning = true;
    chassis.drive_distance(27,180,12,6,1,300,1000);
    wait(0.1,sec);

    chassis.turn_to_angle(45,8,1,300,900);
    intakeRunning = false;
    intak.stop();
    tilt.set(false); //drop G1

   //Step 3: go north to get G2
    chassis.turn_to_angle(270,4,1,300,1000);
    chassis.drive_distance(-19,270,6,6,1,300,900);
    tilt.set(true);
    wait(0.3,sec);

    chassis.drive_distance(40,295,12,10, 1,300,1000);
    chassis.drive_distance(30,192,12,10, 1,300,1200);
    intakeRunning = false;
    intak.stop();
    doink.set(true);
    chassis.drive_distance(10,192,12,10, 1,300,1500);
    chassis.turn_to_angle(110, 12, 0.5, 300, 1500);
    doink.set(false);
  /*  chassis.turn_to_angle(205, 6, 0.5, 300, 800);
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.drive_distance(10,205,12,10, 1,300,3000);
    intak.stop();*/
  }//end of red alliance, score on alliance stake
  else if(current_color_selection==1 && doAllianceStake)
  //blue alliance, score on alliance stake
  //Robot starts facing the rings in front of the alliance stake
  //preload ring on LB arm
  /************************************************** 
  |   G2                                   |
  |                                        |
  |   R1    G1                             |
  |                                        |
  |   R                R                   |
  |BP           Robot  R                 BN|
   -------------------STK----------------------
                 blue alliance
  **************************************************/ 
  {
    ringCatcher.setPosition(0,degrees);
    chassis.turn_to_angle(19, 6, 0.5, 300, 300); //turn to face the alliace stake
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);//return LB arm to starting position
    
    //go get G1
    chassis.drive_distance(-30,90,12, 9, 1,300,1100);
    chassis.drive_distance(-12, 90, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.2,sec);

    //Step 2: go east to get R1 and score onto G1
    chassis.turn_to_angle(180, 6, 0.5, 300, 800);
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.drive_distance(27,180,12,6,1,300,1000);
    wait(0.1,sec);

    chassis.turn_to_angle(315,8,1,300,900);
    intakeRunning = false;
    intak.stop();
    tilt.set(false); //drop G1

    //Step 3: go north to get G2, score R1 into G2
    chassis.turn_to_angle(90,4,1,500,2000);
    chassis.drive_distance(-14,90,6,6,1,300,600);
    chassis.drive_distance(-5, 90, 4, 6, 1, 300, 400);
    tilt.set(true);
    wait(0.2,sec);

    chassis.drive_distance(20,90,8,6, 1,300,1500);
    intakeRunning = false;
    intak.stop();
    chassis.drive_distance(25,110,8,6, 1,300,1500);
    doink.set(true);
    chassis.drive_distance(5,110,12,10, 1,300,500);
    chassis.turn_to_angle(20, 12, 0.5, 300, 800);
    doink.set(false);
  }//end of blue alliance, score on alliance stake
}//end of posSide_elim
/********************************************************/

/********************************************************/
//this auton is for worlds only. not ready to use for states
void middlegoal_wall()
{

  if(current_color_selection==0)//red
  {
    //rearBrake();
    chassis.drive_distance(-16, 0, 6, 6, 1, 300, 800);
    chassis.drive_distance(-13, 0, 4, 6, 1, 300, 800);
    tilt.set(true); //clamp on the goal
    wait(0.3,sec); 

    //score the preload onto the goal
    intak.spin(fwd,12,voltageUnits::volt);
    wait(0.5,sec);
    intakeRunning = true;

    //go get the second ring and store in intake
      chassis.turn_max_voltage = 6;
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(270);
    tilt.set(false); //release first goal
    intak.spin(fwd,9,voltageUnits::volt);
    
    ringCatcherLoad();
    chassis.drive_max_voltage = 12;
    chassis.drive_timeout = 1000;
    chassis.drive_distance(27);
    wait(1.5,sec);
    intakeRunning = false;
    intak.stop();
    chassis.drive_distance(-3);

    //get the middle goal
    chassis.turn_max_voltage = 4;
    chassis.turn_timeout = 1000;
    chassis.turn_to_angle(0);
    chassis.drive_distance(-14,0,6,6,1,300,600);
    chassis.drive_distance(-5, 0, 4, 6, 1, 300, 400);
    tilt.set(true);
    wait(0.3,sec);
    chassis.drive_distance(20,30,6,6,1,300,1500);
    chassis.turn_to_angle(250,6,1,300,2000);

    chassis.drive_distance(15,250,6,6,1,300,5000);
    chassis.turn_to_angle(230);
    /*
    chassis.drive_distance(19,220,6,6,1,300,900);
    chassis.turn_to_angle(225);
    dunk();
    */


    //put ring on goal
  //  intak.spin(fwd,12,voltageUnits::volt);

    //clear the positive corner
  /*  chassis.drive_distance(48, 0, 12, 6, 1, 300, 1200);
    chassis.turn_timeout = 1000;
    chassis.turn_to_angle(300);
    doink.set(true);

    chassis.drive_distance(12,300, 12, 6, 1, 300, 600);
    chassis.turn_max_voltage = 12;
    chassis.turn_to_angle(220);
    doink.set(false);
    */
  }
  else //blue alliance
  {
    rearBrake();
    chassis.drive_distance(-16, 0, 12, 6, 1, 300, 1000);
    chassis.drive_distance(-13, 0, 4, 6, 1, 300, 800);
    tilt.set(true); //clamp on the goal
    wait(0.3,sec); 

    //score the preload onto the goal
    intak.spin(fwd,12,voltageUnits::volt);
    wait(0.5,sec);
    intakeRunning = true;

    //go get the second ring and store in intake
    chassis.turn_max_voltage = 6;
    chassis.turn_timeout = 800;
    chassis.turn_to_angle(100);
    tilt.set(false); //release first goal
    intak.spin(fwd,9,voltageUnits::volt);
    
    chassis.drive_max_voltage = 12;
    chassis.drive_timeout = 1000;
    chassis.drive_distance(27);
    wait(0.1,sec);
    intakeRunning = false;
    intak.stop();
    chassis.drive_distance(-3);

    //get the middle goal
    chassis.turn_max_voltage = 4;
    chassis.turn_timeout = 1000;
    chassis.turn_to_angle(0);
    chassis.drive_distance(-13,0,12,6,1,300,600);
    chassis.drive_distance(-4, 0, 4, 6, 1, 300, 410);
    tilt.set(true);
    wait(0.3,sec);

    //put ring on goal
    intak.spin(fwd,12,voltageUnits::volt);
  
    //clear the corner
    chassis.drive_distance(38, 0, 12, 6, 1, 300, 1200);
    chassis.turn_timeout = 1000;
    chassis.turn_to_angle(36);
    doink.set(true);
    chassis.drive_distance(15,35, 12, 6, 1, 300, 600);
    chassis.turn_max_voltage = 12;
    chassis.turn_to_angle(315);
    doink.set(false);
  }//end of blue
} //end of middleGoal_wall
/*****************************/

/**********************************
This auton is for qualification match, 
when our robot starts on the negative corner side;
user chooses from the brain :
1. red or blue alliance
2. whether or not to score on the alliance stake
***********************************/
void negSide_qual()
{
  if(current_color_selection==0 && !doAllianceStake )
  //red alliance, NOT score on the alliance stake
  //Robot starts in front of the goal, back facing the goal
  //preload ring on the intake
  /**************************************************
  |    R  R                               |
  |    R3 R1                              |
  |                                       |
  |     R2    G1                          |
  |                                       |
  |         robot      R                  |
  |RN                  R                RP|
   -------------------STK----------------------
                 red alliance
  **************************************************/  
  {
    //step 1: robot back going north to get the goal G1
    chassis.drive_distance(-15,0,12,9.1, 1,300,600);
    chassis.drive_distance(-15, 0, 6, 6,1,300,600); 
    tilt.set(true);//clamp the goal
    wait(0.3,sec);
    
    //step 2: go northwest to intake and score R1 into G1
    chassis.turn_to_angle(135, 6, 0.5, 300, 1000);
    intak.spin(fwd,10,voltageUnits::volt);
    chassis.drive_distance(20, 135, 8, 6,1,300,1000); 
    intakeRunning = 1;

    //step 3: go southwest to intake and score R2 into G1
    chassis.drive_distance(-20, 135, 8, 6,1,300,800);
    chassis.drive_distance(25, 90, 12, 12,1,300,1100);

    //step 4: go north to intake and score R3 into G1
    chassis.turn_to_angle(176, 6, 0.5, 300, 800);
    chassis.drive_distance(13, 176, 8, 6,1,300,1000);

    //step 5: go touch the bar for potentail AWP 
    chassis.drive_distance(-30,150,12,10, 1,300,1500);
    chassis.turn_to_angle(225);
    intakeRunning = false;
    intak.stop();
    dunk();
    //ringCatcher.spinToPosition(900,degrees,false);
    chassis.drive_distance(18, 225, 8, 6, 1, 300, 1500);

  } // end of red, no alliance stake
  else if(current_color_selection==1 && !doAllianceStake) 
  //blue alliance, NOT score on the alliance stake
  //Robot starts in front of the goal, back facing the goal
  //preload ring on the intake
  /**************************************************
  |                                   R  R  |
  |                                   R3 R1 |
  |                                         |
  |                              G1    R2   |
  |                                         |
  |                    R       robot        |
  |BP                  R                  BN|
   -------------------STK----------------------
                 blue alliance
  **************************************************/  
  {
    //step 1: robot back going north to get the goal G1
    chassis.drive_distance(-15,0,12,9.1, 1,300,600);
    chassis.drive_distance(-15, 0, 6, 6,1,300,600); 
    tilt.set(true);//clamp the goal
    wait(0.3,sec);
    
    //step 2: go northwest to intake and score R1 into G1
    chassis.turn_to_angle(225, 6, 0.5, 300, 1000);
    intak.spin(fwd,11,voltageUnits::volt);
    chassis.drive_distance(20, 225, 8, 6,1,300,1000); 
    intakeRunning = 1;

    //step 3: go southwest to intake and score R2 into G1
    chassis.drive_distance(-20, 225, 8, 6,1,300,800);
    chassis.drive_distance(25, 270, 8, 8,1,300,1100);

    //step 4: go north to intake and score R3 into G1
    chassis.turn_to_angle(184, 6, 0.5, 300, 1000);
    chassis.drive_distance(13, 184, 8, 6,1,300,1000);

    //step 5: go touch the bar for potentail AWP 
    chassis.drive_distance(-30,210,12,10, 1,300,1500);
    chassis.turn_to_angle(135);
    intakeRunning = false;
    intak.stop();
    ringCatcher.spinToPosition(800,degrees,false);
    chassis.drive_distance(18, 135, 8, 6, 1, 300, 1500);
  } //end of blue, no alliance stake
  else if(current_color_selection==0 && doAllianceStake) 
  //red alliance, score on the alliance stake
  //Robot starts facing the rings in front of the alliance stake
  //use measuring channel to find the starting position
  //preload ring on the LB arm
  /**************************************************
  |    R  R                               |
  |    R3 R1                              |
  |                                       |
  |     R2    G1                          |
  |                                       |
  |                    R                  |
  |RN            robot R                RP|
   -------------------STK----------------------
                 red alliance
  **************************************************/ 
  {
    ringCatcher.setPosition(0,degrees);
    chassis.turn_to_angle(20, 6, 0.5, 300, 300); //turn to face the alliace stake
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);//return LB arm to starting position
    
    chassis.drive_distance(-30,90,12, 9, 1,300,1100);
    chassis.drive_distance(-12, 90, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.3,sec);

    //step 2: go northwest to intake and score R1 into G1
    chassis.turn_to_angle(225, 6, 0.5, 300, 1000);
    intak.spin(fwd,10,voltageUnits::volt);
    chassis.drive_distance(19, 225, 8, 6,1,300,1000); 
    intakeRunning = 1;

    //step 3: go southwest to intake and score R2 into G1
    chassis.drive_distance(-20, 225, 8, 6,1,300,800);
    chassis.drive_distance(25, 180, 12, 12,1,300,1100);

    //step 4: go north to intake and score R3 into G1
    chassis.turn_to_angle(260, 6, 0.5, 300, 800);
    chassis.drive_distance(13, 260, 8, 6,1,300,1000);

    //step 5: go touch the bar for potentail AWP 
    chassis.drive_distance(-30,240,12,10, 1,300,1500);
    chassis.turn_to_angle(315, 6, 0.5, 300, 800);
    intakeRunning = false;
    intak.stop();
    ringCatcher.setVelocity(100,percent);
    ringCatcher.spinToPosition(800,degrees,false);
    chassis.drive_distance(19, 315, 8, 6, 1, 300, 1500);
    
  }//end of red, score alliance stake
  else if(current_color_selection==1 && doAllianceStake) 
  //blue alliance, score on the alliance stake
  //Robot starts facing the rings in front of the alliance stake
  //use measuring channel to find the starting position
  //preload ring on the LB arm
  /**************************************************
  |                                   R  R  |
  |                                   R3 R1 |
  |                                         |
  |                              G1    R2   |
  |                                         |
  |                    R                    |
  |BP                  R robot            BN|
   -------------------STK----------------------
                 blue alliance
  **************************************************/  
  {
    ringCatcher.setPosition(0,degrees);
    chassis.turn_to_angle(340, 6, 0.5, 300, 300); //turn to face the alliace stake
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);
    
    chassis.drive_distance(-30,270,12, 8.5, 1,300,1100);
    chassis.drive_distance(-12, 270, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.3,sec);

    //step 2: go northeast to intake and score R1 into G1
    chassis.turn_to_angle(135, 6, 0.5, 300, 1000);
    intak.spin(fwd,10,voltageUnits::volt);
    chassis.drive_distance(19, 135, 8, 6,1,300,1000); 
    intakeRunning = 1;

    //step 3: go southeast to intake and score R2 into G1
    chassis.drive_distance(-20, 135, 8, 6,1,300,800);
    chassis.drive_distance(25, 180, 12, 12,1,300,1100);

    //step 4: go north to intake and score R3 into G1
    chassis.turn_to_angle(100, 6, 0.5, 300, 800);
    chassis.drive_distance(13, 100, 8, 6,1,300,1000);

    //step 5: go touch the bar for potentail AWP 
    chassis.drive_distance(-30,120,12,10, 1,300,1400);
    chassis.turn_to_angle(45);
    intakeRunning = false;
    intak.stop();
    ringCatcher.setPosition(0,degrees);
    ringCatcher.spinToPosition(800,degrees,false);
    chassis.drive_distance(23, 45, 8, 6, 1, 300, 2500);
  }//end of blue, score alliance stake
}//end of negSide_qual
/******************************************/

/**********************************
This auton is for elimination match. 
This auton is when our robot starts on the negative corner side;
This auton does NOT touch the bar
***********************************/
void negSide_elim()
{
  if(current_color_selection==0 && !doAllianceStake) 
  {
    /**************************************************
    |    R  R                               |
    |    R3 R1                              |
    |                                       |
    |     R2    G1                          |
    |                                       |
    |         robot      R                  |
    |RN                  R                RP|
     -------------------STK----------------------
                   red alliance
    **************************************************/
    //red alliance, do NOT score on alliance stake
    //Robot starts in front of the goal, back facing the goal
    //preload ring on the intake

    //step 1: robot back going north to get the goal G1
    chassis.drive_distance(-15,0,12,9.1, 1,300,600);
    chassis.drive_distance(-15, 0, 6, 6,1,300,600); 
    tilt.set(true);//clamp the goal
    wait(0.3,sec);
    
    //step 2: go northwest to intake and score R1 into G1
    chassis.turn_to_angle(135, 6, 0.5, 300, 900);
    intak.spin(fwd,10,voltageUnits::volt);
    chassis.drive_distance(20, 135, 8, 6,1,300,1000); 
    intakeRunning = 1;

    //step 3: go southwest to intake and score R2 into G1
    chassis.drive_distance(-20, 135, 8, 6,1,300,800);
    chassis.drive_distance(26, 90, 8, 8,1,300,1100);

    //step 4: go north to intake and score R3 into G1
    chassis.turn_to_angle(180, 6, 0.5, 300, 800);
    chassis.drive_distance(11, 180, 8, 6,1,300,1000);

    //step 5: go to the red negative(RN) corner 
    //and use doinker to remove the top 3 rings from th corner
    chassis.turn_to_angle(0, 6, 0.5, 300, 1000);
    chassis.drive_distance(40, 0, 8, 6,1,300,1200);
    intakeRunning = 0;
    intak.stop(); 
    chassis.turn_to_angle(37, 6, 0.5, 300, 600);
    doink.set(true);
    chassis.drive_distance(10, 37, 8, 6,1,300,600);
    chassis.turn_to_angle(300, 12, 0.5, 300, 1200);
    doink.set(false);

    //step 6: intake and score the bottom ring from the RN corner
  /*  chassis.drive_distance(-10, 24, 6, 12, 1,300,1500);
    intak.spin(fwd,12,voltageUnits::volt);
    chassis.drive_distance(15, 24, 8, 4,1,300,1000);*/
  } // end of red, no alliance stake
  else if(current_color_selection==1 && !doAllianceStake) 
  {
    /**************************************************
    |                                   R  R  |
    |                                   R3 R1 |
    |                                         |
    |                              G1    R2   |
    |                                         |
    |                    R       robot        |
    |BP                  R                  BN|
     -------------------STK----------------------
                   blue alliance
    **************************************************/
    //blue alliance, do NOT score on alliance stake
    //Robot starts in front of the goal, back facing the goal
    //preload ring on the intake

    //step 1: robot back going north to get the goal G1
    chassis.drive_distance(-15,0,12,9.1, 1,300,600);
    chassis.drive_distance(-15, 0, 6, 6,1,300,600); 
    tilt.set(true);//clamp the goal
    wait(0.3,sec);
    
    //step 2: go northwest to intake and score R1 into G1
    chassis.turn_to_angle(225, 6, 0.5, 300, 1000);
    intak.spin(fwd,11,voltageUnits::volt);
    chassis.drive_distance(20, 225, 8, 6,1,300,1000); 
    intakeRunning = 1;

    //step 3: go southwest to intake and score R2 into G1
    chassis.drive_distance(-20, 225, 8, 6,1,300,800);
    chassis.drive_distance(26, 270, 8, 8,1,300,1100);

    //step 4: go north to intake and score R3 into G1
    chassis.turn_to_angle(182, 6, 0.5, 300, 1000);
    chassis.drive_distance(11, 182, 8, 6,1,300,1000);

    //step 5: go to the blue negative(BN) corner and use doinker to clear the corner
    chassis.turn_to_angle(0, 6, 0.5, 300, 1100);
    chassis.drive_distance(45, 0, 12, 6,1,300,1300);
    intakeRunning = 0;
    intak.stop(); 
    chassis.turn_to_angle(310, 6, 0.5, 300, 800);
    doink.set(true);
    chassis.drive_distance(8, 310, 8, 6,1,300,600);
    chassis.turn_to_angle(210, 12, 0.5, 300, 800);
    doink.set(false);
  /*  chassis.drive_distance(-10, 310, 6, 12, 1,300,1300);
    intak.spin(fwd,10,voltageUnits::volt);
    chassis.drive_distance(15, 310, 8, 4,1,300,1000);*/
  } // end of blue, no alliance stake
  else if(current_color_selection==0 && doAllianceStake) 
  {
    /**************************************************
    |    R  R                               |
    |    R3 R1                              |
    |                                       |
    |     R2    G1                          |
    |                                       |
    |         robot      R                  |
    |RN                  R                RP|
     -------------------STK----------------------
                   red alliance
    **************************************************/
    //red alliance, do NOT score on alliance stake
    //Robot starts in front of the goal, back facing the goal
    //preload ring on the intake

    ringCatcher.setPosition(0,degrees);
    chassis.turn_to_angle(20, 6, 0.5, 300, 300); //turn to face the alliace stake
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);//return LB arm to starting position
    
    chassis.drive_distance(-30,90,12, 9, 1,300,1100);
    chassis.drive_distance(-12, 90, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.3,sec);

    //step 2: go northwest to intake and score R1 into G1
    chassis.turn_to_angle(225, 6, 0.5, 300, 1000);
    intak.spin(fwd,10,voltageUnits::volt);
    chassis.drive_distance(19, 225, 8, 6,1,300,1000); 
    intakeRunning = 1;

    //step 3: go southwest to intake and score R2 into G1
    chassis.drive_distance(-20, 225, 8, 6,1,300,800);
    chassis.drive_distance(25, 180, 12, 12,1,300,1100);

    //step 4: go north to intake and score R3 into G1
    chassis.turn_to_angle(260, 6, 0.5, 300, 800);
    chassis.drive_distance(13, 260, 8, 6,1,300,1000);
    wait(0.5,sec);
    intakeRunning = false;
    intak.stop();

    //step 5: go to the red negative(RN) corner 
    //and use doinker to remove the top 3 rings from th corner
    chassis.turn_to_angle(90, 6, 0.5, 300, 1000);
    chassis.drive_distance(40, 90, 8, 6,1,300,1200);
    intakeRunning = 0;
    intak.stop(); 
    chassis.turn_to_angle(127, 6, 0.5, 300, 600);
    doink.set(true);
    chassis.drive_distance(10, 127, 8, 6,1,300,600);
    chassis.turn_to_angle(30, 12, 0.5, 300, 1200);
    doink.set(false);
  } //end of red, with alliance stake
  else if(current_color_selection==1 && doAllianceStake) 
  {
    /**************************************************
    |                                   R  R  |
    |                                   R3 R1 |
    |                                         |
    |                              G1    R2   |
    |                                         |
    |                    R0      robot        |
    |BP                  R0                 BN|
     -------------------STK----------------------
                   blue alliance
    **************************************************/
    //blue alliance, do NOT score on alliance stake
    //Robot starts facing the ring R0
    //preload ring on the LB ear
    ringCatcher.setPosition(0,degrees);
    chassis.turn_to_angle(340, 6, 0.5, 300, 300); //turn to face the alliace stake
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);
    
    chassis.drive_distance(-30,270,12, 8.5, 1,300,1100);
    chassis.drive_distance(-12, 270, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.3,sec);

    //step 2: go northeast to intake and score R1 into G1
    chassis.turn_to_angle(135, 6, 0.5, 300, 1000);
    intak.spin(fwd,10,voltageUnits::volt);
    chassis.drive_distance(19, 135, 8, 6,1,300,1000); 
    intakeRunning = 1;

    //step 3: go southeast to intake and score R2 into G1
    chassis.drive_distance(-20, 135, 8, 6,1,300,800);
    chassis.drive_distance(25, 180, 12, 12,1,300,1100);

    //step 4: go north to intake and score R3 into G1
    chassis.turn_to_angle(100, 6, 0.5, 300, 800);
    chassis.drive_distance(13, 100, 8, 6,1,300,1000);

    //step 5: go to the blue negative(BN) corner and use doinker to clear the corner
    chassis.turn_to_angle(270, 6, 0.5, 300, 1100);
    chassis.drive_distance(45, 270, 12, 6,1,300,1300);
    intakeRunning = 0;
    intak.stop(); 
    chassis.turn_to_angle(215, 6, 0.5, 300, 800);
    doink.set(true);
    chassis.drive_distance(8, 215, 8, 6,1,300,600);
    chassis.turn_to_angle(120, 12, 0.5, 300, 800);
    doink.set(false);
  } //end of blue, with alliance stake
}//end of negSide_elim
/******************************************/

/****************AWP solo for state ************/
//this auton is used when the alliance does not have auton
//it will score 4 rings onto 3 goals to get AWP at states
//ADD FIELD MAP here
void AWP_solo()
{
   //swing lady brown arm to score the preload onto the alliance stake
    ringCatcher.setPosition(0, degrees);
    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(1.5, sec);
    ringCatcher.spinToPosition(850,degrees,true);
    ringCatcher.setVelocity(50,percent);
    ringCatcher.spinToPosition(0,degrees, false);
    
    chassis.drive_distance(-30,70,12, 9, 1,300,1100);
    chassis.drive_distance(-12, 70, 6, 6,1,300,600); 
    ringCatcher.setVelocity(100,percent);
    tilt.set(true); //clamp the goal G1
    wait(0.3,sec);
    
    chassis.turn_to_angle(160, 6, 0.5, 300, 700);
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    chassis.drive_distance(25, 160, 12, 6,1,300,1000); 
    intakeRunning = true;

    //go back to drop the goal
    chassis.drive_distance(-36,250,12,9.1,1,300,1200);
    tilt.set(false);

    //go to get middle rings
    chassis.turn_to_angle(340, 6, 0.5, 300, 800);
    intak.spin(fwd,12,voltageUnits::volt);
    intakeRunning=true;
    chassis.drive_distance(27,340,12,9.1,1,300,1000);//27
    chassis.drive_distance(13,340,12,9.1,1,300,600);
    intak.spin(fwd,4,voltageUnits::volt);
    chassis.drive_distance(20,340,12,9.1,1,300,800);//20
    intakeRunning = false;
    intak.stop();
      
    //go to get the 2nd goal
    chassis.turn_to_angle(70, 6, 0.5, 300, 800);

    //drive back and clamp goal changing
    chassis.drive_distance(-20,65,12,9.1,1,300,700 );
    chassis.drive_distance(-12,65,7,6,1,300,300);
    tilt.set(true);
    wait(0.2,sec);
    
    //get ring
    intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);
    intakeRunning = true;
    chassis.turn_to_angle(340, 6, 0.5, 300, 700);
    chassis.drive_distance(25,340,12,6,1,300,1200);

    // touch bar
    chassis.drive_distance(-30,270,12,5,1,300,1200);
    intakeRunning = false;
    intak.stop();
    ringCatcher.spinToPosition(800,degrees, false);
    chassis.drive_distance(10,220,6,6,1,300,1000);
} // end of the AWP solo for state
/****************end of the AWP solo for state ************/


/***************** auton skills *****************/
void skills()
{
  //Robot starts in front of the alliance stake, facing north, 7 holes away from the wall
  //pre-load on the intake 
  
  coastBrake(); //set drivetrain motor brake to coast mode

  /////////////////////////////////////////////////
  //Step 1: spin intake to put the preload onto alliance stake
  /////////////////////////////////////////////////
  intak.spin(fwd,12,voltageUnits::volt);
  wait(0.3,sec);
  intak.stop();

  /////////////////////////////////////////////////
  //Step 2: the red positive(RP) corner side of the field     
  //              R1     R2      |
  //                             |
  //              G1     R3 R5   | 
  //     Robot           R4   RP |
  //------STK------------------------
  /////////////////////////////////////////////////

  //get mobile goal G1
  chassis.drive_distance(14, 0, 12, 6, 1, 300, 700);
  chassis.turn_to_angle(270, 6, 0.5, 300, 700);
  chassis.drive_distance(-10, 270, 12, 6, 1, 300, 500);
  chassis.drive_distance(-10, 270, 6, 6, 1, 300, 600);//-16.5
  tilt.set(true);
  wait(0.3,sec);
  chassis.drive_distance(-6, 270, 12, 6, 1, 300, 500);

  //move north to intake R1 and put into G1
  intak.spin(fwd,10,voltageUnits::volt);
  chassis.turn_to_angle(0, 6, 0.5, 300, 700);
  intakeRunning = true;
  chassis.drive_distance(23, 0, 8, 6, 1, 300, 1000);
  wait(0.3,sec);
  
  //move east to intake R2 and put into G1
  chassis.turn_to_angle(90, 6, 0.5, 300, 800);
  chassis.drive_distance(30, 90, 8, 6, 1, 300, 1500);
  chassis.drive_distance(-6);

  //move south to intake R3 and R4 and put into G1
  chassis.turn_to_angle(180, 6, 0.5, 300, 700);
  chassis.drive_max_voltage = 8;
  chassis.drive_distance(21,180,8,6,1,300,800);
  wait(0.5,sec);
  chassis.drive_distance(14,180,8,6,1,300,600); 
  wait(0.5,sec);

  //move northeast to intake R5 and put into G1
  chassis.turn_to_angle(42,6, 0.5, 300, 700);
  chassis.drive_distance(16,42,8,6,1,300,600);
  wait(0.5,sec);

  //turn northwest and put G1 in the red positive (RP) corner
  chassis.turn_to_angle(345,6, 0.5, 300, 500);
  chassis.drive_distance(-12,345,8,6,1,300,600);
  intakeRunning=false;
  intak.stop();
  tilt.set(false);
  
  //move out of the RP corner
  chassis.drive_distance(9,345,8,6,1,300,400); 

  /***** Step 3: move to the red negative(RN) side of the field
  |       R7    R6                            |
  |                                           |
  |   R10 R8    G2                    Robot   |
  |RN     R9                                RP|
   -------------------STK----------------------
                 red alliance
  **************************************************/
 
  //turn to face west
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(270);
  intak.spin(fwd,10,voltageUnits::volt);//spin intake to get rid of the rings on the way
  intakeRunning = true;
  //move west to get G2
  chassis.drive_distance(60, 270, 8, 6, 1, 300, 5000);
  chassis.turn_to_angle(90, 6, 0.5, 300, 1000);
  chassis.drive_distance(-28, 90, 4, 6, 1, 300, 2500); 
  tilt.set(true);
  wait(0.3,sec);
  chassis.drive_distance(4, 90, 6, 6, 1, 300, 2000); 
 
  //move north to intake R6 and put into G2
  chassis.turn_to_angle(0, 6, 0.5, 300, 800);
  chassis.drive_distance(23, 0, 8, 6, 1, 300, 1200);
  wait(0.5,sec);
  
  //move west to intake R7 and put into G2
  chassis.turn_to_angle(270, 6, 0.5, 300, 900);
  chassis.drive_distance(24, 270, 8, 6, 1, 300, 1000);
  wait(0.2,sec);
  
  //move south to intake R8 and R9 and put into G2
  chassis.turn_to_angle(180, 6, 0.5, 300, 900);
  chassis.drive_distance(21,180, 8, 6, 1, 300, 1000);
  wait(0.3,sec);
  chassis.drive_distance(12,180, 8, 6, 1, 300, 600);

  //move northwest to intake R10 and put into G2
  chassis.turn_to_angle(317, 6, 0.5, 300, 1000);
  chassis.drive_distance(12,317,8,6,1,300,500);
  wait(0.5,sec);

  //put G2 in the red negative(RN) corner
  chassis.turn_to_angle(20, 6, 0.5, 300, 800);
  chassis.drive_distance(-10, 25, 12, 6, 1, 300, 800);
  intakeRunning=false;
  intak.stop();
   
  tilt.set(false); //release the goal
  wait(0.2,sec);
  chassis.drive_distance(32, 25, 12, 6, 1, 300, 1000); //get out of the corner

  /////////////////////////////////////////////////
  /****  Step 4: go to the blue negative(BN) side
  get the goal G3 and put it into the BN corner
  use intake to remove the rings on the way to the BN corner
           blue alliance
  --------------STK-------------
  |BN R     G3    
  | R R         G4                     
  |              
  |   R     R            
  |       
  | Robot        
  |       
  |RN  
  --------------STK--------------     
           red alliance            
  *******************************************/

  chassis.turn_to_angle(0, 6, 0.5, 300, 600);
  intak.spin(fwd,12,voltageUnits::volt);
  intakeRunning=true;
  chassis.drive_distance(61, 0, 8, 6, 1, 300, 3500);

  //get the mobile goal G3 on the BN side (with blue ring on it)
  chassis.turn_to_angle(40, 6, 0.5, 300, 550);
  chassis.drive_distance(22, 40, 12, 6, 1, 300, 750);
  intakeRunning=false;
  intak.stop();
  chassis.turn_to_angle(220, 6, 0.5, 300, 800);
  chassis.drive_distance(-18, 220, 6, 6, 1, 300, 1000); 
  tilt.set(true); //clamp the goal
  wait(0.3,sec);
  
  //put G3 into the BN corner
  chassis.turn_to_angle(270, 6, 0.5, 500, 700);
  //spin intake to remove the rings on the way to the BN corner
  intak.spin(fwd,12,voltageUnits::volt);
  intakeRunning = true;
  chassis.drive_distance(30, 270, 8, 6, 1, 300, 1300);
  chassis.turn_to_angle(150, 12, 0.5, 500, 1200);
  chassis.drive_distance(5, 150, 12, 6, 1, 300, 400);
  tilt.set(false); //release goal
  intakeRunning=false;
  intak.stop();
  chassis.drive_distance(-8, 150, 12, 6, 1, 300, 500);
 
  /***** Step 5: get the empty mobile goal G4 in the middle of the blue side
  intake and score R11 and R12 into G4 
  put G4 into BP corner
           blue alliance
  --------------STK------------- 
  |BN       G3      G      R BP|
  |  Robot      G4         R R |         
  |                            |
  |   R     R       R11    R12 |   
  ************************************************/

  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_distance(11, 150, 12, 6, 1, 300, 500);
  intakeRunning = true;
  chassis.turn_to_angle(90, 6, 0.5, 500, 700);
  chassis.drive_distance(32, 90, 8, 6, 1, 300, 3000);
  chassis.turn_to_angle(270, 6, 0.5, 500, 1000);
  chassis.drive_distance(-29,270, 4, 6, 1, 300, 2000); 
  tilt.set(true);
  wait(0.3,sec);

  //get R11 and put into G4
  chassis.turn_to_angle(150, 6, 0.5, 500, 1200);
  chassis.drive_distance(28, 150, 8, 6, 1, 300, 1300); 
  //get R12 and put into G4
  chassis.turn_to_angle(80, 6, 0.5, 500, 1000);
  chassis.drive_distance(35, 80, 12, 6, 1, 300, 1000); 

  //put the goal into the blue positive (BP) corner
  chassis.turn_to_angle(0, 6, 0.5, 500, 800);
  chassis.drive_distance(30, 0, 8, 6, 1, 300, 1500);
  chassis.turn_to_angle(250, 12, 0.5, 500, 1500);
  tilt.set(false);
  intakeRunning = 0;
  intak.stop();
//  chassis.drive_distance(-10,240,12,6,1,300,800);


  //re-push G4 into the BP corner
  chassis.drive_distance(5,240,12,6,1,300,400);
  chassis.drive_distance(-10,240,12,6,1,300,400);
  chassis.drive_distance(10);
}// end of skills()

void driveTest()
{
  //intak.spin(fwd,12,voltageUnits::volt);
   chassis.turn_to_angle(20, 6, 0.5, 300, 300); //turn to face the alliace stake
//  coastBrake();
//    chassis.turn_to_angle(90,6,0.5,500,1000);
  // stopBrake();
//   chassis.drive_distance(60,90,12,6,1,300,2000);
//    ringCatcher.spinToPosition(550,degrees,true);
    //dunk(); // put preload onto the alliance stake
//    ringCatcher.setVelocity(30,percent);
//    ringCatcher.spinToPosition(0,degrees, false);
 //
}
void LBtest()
{
    //ringCatcher.setPosition(0, degrees);
    //chassis.turn_timeout =300;
    //chassis.turn_to_angle(25);//16

    ringCatcher.setVelocity(100,percent);
    ringCatcher.setTimeout(2, sec);
    cout<<"position 1 = "<<ringCatcher.position(degrees)<<endl;
    ringCatcher.spinToPosition(130,degrees,true);
    cout<<"position 2 = "<<ringCatcher.position(degrees)<<endl;
   
      cout<<"position 3 = "<<ringCatcher.position(degrees)<<endl;
      wait(2,sec);
       ringCatcher.spinToPosition(00,degrees,true);
   wait(2,sec);
      cout<<"position 4 = "<<ringCatcher.position(degrees)<<endl;
  //  ringCatcherReset();
}

void ringsortingtest()
{
  tilt.set(true);
  intak.spin(fwd, intakeVoltage_scoringGoals,voltageUnits::volt);
  intakeRunning = true;
}



void autonomous(void) {
  auto_started = true;
  if (auto_started)
  {
    Brain.Screen.printAt(50, 50, "neg_elim");
    Controller1.Screen.print("neg_elim");
    skills(); //slot 1
    //AWP_solo();//slot 2, tested
    //negSide_qual(); //slot 3
    //posSide_qual(); //slot 4
    //negSide_elim(); //slot 5
    //posSide_elim(); //slot 6
    //ringsortingtest();// slot8
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

void intakeJam()
{
 if (intakeRunning & ringColorCorrect) {
      if (fabs(static_cast<float>(intak.velocity(percent))) < 5.0) {
         foundIntakeJam = true;
      intak.spin(reverse, 12,voltageUnits::volt);
      wait(0.2, seconds);
      intak.spin(fwd, 12 ,voltageUnits::volt);
      wait(0.5, sec);
      foundIntakeJam = false;
    }
  }
}

/************************/
void loadingLB()
{
    if(ringColorSensor.isNearObject())
    {
      loadingRingInProgress = 1;
      wait(0.5,sec);
      ringStopper.set(true);
      ringStopperOut = 1; 
      ringCatcher.spin(fwd, 8, voltageUnits::volt);
      wait(0.3,sec);
      ringCatcher.stop();
      inLoadingPosition = 0;
      loadingRingInProgress = 0;
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

/*********toggle the ringStopper options*********/
void toggle_ringStopper()
{
  if (ringStopperOut) {ringStopper.set(false); ringStopperOut = false;}
  else { ringStopper.set(true); ringStopperOut = true;}
}// end of toggle_ringStopper()



void sortingRingColors()
{
  if (current_color_selection == 0) //red alliance, sort out blue ring
  { 
    if (ringColorSensor.isNearObject() && ringColorSensor.color() == blue) 
    {
      cout<<"blue ring found\n";
      ringColorCorrect = false;
      wait(180,msec);
      intak.stop();
      wait(500,msec);
      if (intakeRunning) { intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);}
      wait(500,msec);
      ringColorCorrect = true;
    }
  }// end of red
  else if (current_color_selection == 1) //blue alliance, sort out red ring
  {
    if (ringColorSensor.isNearObject() && ringColorSensor.color() == red) 
    {
      cout<<"red ring found\n";
      ringColorCorrect = false;
      wait(180,msec);
      intak.stop();
      wait(300,msec);
      if (intakeRunning) { intak.spin(fwd,intakeVoltage_scoringGoals,voltageUnits::volt);}
      wait(500,msec);
      ringColorCorrect = true;
    } //blue alliance
  }// end of red
}

void usercontrol(void) 
{
  //ringColorSensor.setLight(ledState::on); //turn on the optical sensor light
  //ringColorSensor.setLightPower(100, percent);
  // User control code here, inside the loop
  while (1) {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.
    if(Controller1.ButtonR1.pressing())
    {
      if (loadingIntake ) {intak.spin(fwd,intakeVoltage_loadingLB, voltageUnits::volt);}
      else 
      { 
        if(ringColorCorrect)
        {
          intak.spin(fwd,intakeVoltage_scoringGoals, voltageUnits::volt); 
        }
      }
    }
    else if(Controller1.ButtonR2.pressing()){
      if (loadingIntake) {intak.spin(reverse,intakeVoltage_loadingLB, voltageUnits::volt);}
      else { intak.spin(reverse,intakeVoltage_scoringGoals, voltageUnits::volt); }
      
    }
    else if(!loading)
    {
      intak.stop();
    }
 
    if(Controller1.ButtonL1.pressing()){  // ring catcher going up
     
        if(ringCatcherRotation.position(degrees)<230)
         { ringCatcher.spin(fwd,12,voltageUnits::volt);}
    } //L1 pressing
    else if(Controller1.ButtonL2.pressing())//ring catcher going down
    {  
      {ringCatcher.spin(reverse,2,voltageUnits::volt);}
    }// L2 pressing
    else if(!loading && !dunking &&!inLoadingPosition)
    {
      ringCatcher.stop();
    } 
    
    int a3 = Controller1.Axis3.position();
    int a2 = Controller1.Axis2.position();
    if (fabs(a2)>15) {cout<<"a2="<<a2<<" a3="<<a3<<"\n";}
    if (fabs(a3)>  10|| fabs(a2)> 15) //to avoid joystick drift and random touch
    {
      int leftSpeed = 0;
      if (fabs(a3)>10) { leftSpeed = (a3-10)*10/9;}
      int rightSpeed = 0; 
      if (fabs(a2)>15) { rightSpeed = (a2-15)*100/85;}
      runChassis(leftSpeed,rightSpeed);
    }
    else
    {
      Chicken.stop(brakeType::coast);
    }
    if(Controller1.ButtonRight.pressing() && Controller1.ButtonY.pressing())
    {
      intak.stop();
    }
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
  LBLoadingCheck(loadingLB);
  Controller1.ButtonA.pressed(toggleTilt);
  Controller1.ButtonY.pressed(printTemps);
  Controller1.ButtonUp.pressed(dunk);
  Controller1.ButtonDown.pressed(ringCatcherReset);
  Controller1.ButtonRight.pressed(ringCatcherLoad);
  //Controller1.ButtonUp.pressed(toggle_colorSorting);// button up pressed to turn on/off color sorting
  Controller1.ButtonX.pressed(toggle_ringStopper);
  Controller1.ButtonB.pressed(toggle_fan);// 
  
  
  // Prevent main from exiting with an infinite loop.
  while (true) {
    if (!foundIntakeJam && ringColorCorrect) 
    {
      checkIntake.broadcast();
    }
    if (ringColorSorting && ringColorCorrect) 
    {  
      //Brain.Screen.print("ring color check");
     //cout<<"ring color check\n";
     // Brain.Screen.newLine();
      ringColorCheck.broadcast();  
    }
    if (inLoadingPosition & !loadingRingInProgress)
    {
      LBLoadingCheck.broadcast();
    }
    wait(10, msec);
  }
}