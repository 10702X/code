#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// br                   motor         18              
// fr                   motor         19              
// bl                   motor         13              
// fl                   motor         11              
// mr                   motor         16              
// ml                   motor         14              
// intak                motor         20              
// tilt                 digital_out   H               
// Lift                 motor         15              
// ringColorSensor      optical       17              
// elevation            digital_out   G               
// ---- END VEXCODE CONFIGURED DEVICES ----


using namespace vex;

// A global instance of competition
competition Competition;

inertial Inertial = inertial(PORT3);
vex::distance RingDistance = vex::distance(PORT1); //somehow, adding this to device list makes error
vex::distance backDistance = vex::distance(PORT10);
vex::distance sideDistance = vex::distance(PORT9);

motor_group leftDrive (fl, ml, bl);
motor_group rightDrive (fr, mr, br);
smartdrive Chicken(leftDrive, rightDrive, Inertial, 12.56,12.75,10.5,distanceUnits::in);

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
motor_group(fl, ml, bl),

//Right Motors:
motor_group(fr, mr, br),

//Specify the PORT NUMBER of your inertial sensor, in PORT format (i.e. "PORT1", not simply "1"):
PORT3,

//Input your wheel diameter. (4" omnis are actually closer to 4.125"):
4.125,

//External ratio, must be in decimal, in the format of input teeth/output teeth.
//If your motor has an 84-tooth gear and your wheel has a 60-tooth gear, this value will be 1.4.
//If the motor drives the wheel directly, this value is 1:
0.5,

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
bool auto_started = false; //for competition
//bool auto_started = true; //for skills and practice
bool intakeStop = true;
bool outtakeStop = true;
bool ringColorSorting = true; 
//true: use color sorting in driver control 
//false: not to use color sorting in driver control

void printTemps() //print each motor temperature 
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
      Brain.Screen.print(" Lift T=");
      Brain.Screen.print(Lift.temperature(celsius));
      Brain.Screen.print(" intake T=");
      Brain.Screen.print(intak.temperature(celsius));
     
}//end of printTemps()

/**********************************************************************/
void runChassis (int leftSpeed, int rightSpeed)
{
  //set the wheel motor torque to max
  
  bl.setMaxTorque(100,pct);
  ml.setMaxTorque(100,pct);
  fl.setMaxTorque(100,pct);
  br.setMaxTorque(100,pct);
  mr.setMaxTorque(100,pct);
  fr.setMaxTorque(100,pct);
  leftDrive.spin(fwd, leftSpeed, pct);
  rightDrive.spin(fwd, rightSpeed, pct);
}// end of runChassis

void frontBrake()
{
  bl.stop(coast);
  br.stop(coast);
  ml.stop(coast);
  mr.stop(coast);
  fl.stop(brake);
  fr.stop(brake);
} //end of frontBrake()

void rearBrake()
{
  bl.stop(brake);
  br.stop(brake);
  ml.stop(brake);
  mr.stop(brake);
  fl.stop(coast);
  fr.stop(coast);
}//end of rearBrake()

void stopBrake()
{
  bl.stop(hold);
  br.stop(hold);
  ml.stop(hold);
  mr.stop(hold);
  fl.stop(hold);
  fr.stop(hold);
}//end of rearBrake()

void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  Brain.Screen.setFont(vex::fontType::mono60);

  vexcodeInit();
  Inertial.calibrate();
  default_constants();
  tilt.set(false);
  Lift.setPosition(0, degrees);

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

/*********AWP_solo_rightside**********/
//This works for both RED and BLUE alliance right side
void AWP_solo_rightside()
{
  //setup voltage, timeout and brake mode
  chassis.drive_max_voltage = 12;
  chassis.turn_max_voltage = 6;
  chassis.drive_timeout = 980;
  chassis.turn_timeout = 550;
  frontBrake();
  Lift.setBrake(hold);
  //push the blue ring back while lifting arm
  Lift.spin(fwd,100,velocityUnits::pct);
  chassis.drive_distance(-30);
  Lift.stop();

  //go intake the first ring
  chassis.turn_to_angle(40);
  chassis.turn_timeout = 1500;
  intak.spin(fwd,10,voltageUnits::volt);
  chassis.drive_max_voltage = 5;
  chassis.drive_distance(23);
  wait(0.2,sec);
  chassis.drive_max_voltage = 12;
  chassis.drive_distance(15);
  wait(0.4,sec);
  intak.stop();
 
  //go put the preload onto alliance stake
  chassis.drive_timeout = 700;
  chassis.drive_distance(-23);//change
  chassis.turn_timeout = 1200;
  chassis.turn_to_angle(270);//turn to face the alliance stake
  chassis.drive_timeout = 500;
  chassis.drive_distance(11);
  Lift.setTimeout(2,sec);
  Lift.spin(reverse,12,voltageUnits::volt);
  wait(0.2,sec);
  chassis.drive_timeout = 200;
  chassis.drive_distance(-4.0);
  wait(0.3,sec);
  Lift.spin(fwd,10,voltageUnits::volt);
  wait(0.1,sec);
  Lift.stop();

  //go grab the mobile goal
  chassis.drive_timeout = 500;
  chassis.drive_distance(-3.0);
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(304);
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 800;
  chassis.drive_distance(-31);
  chassis.drive_max_voltage = 6;
  chassis.drive_timeout = 800;
  chassis.drive_distance(-15);
  tilt.set(true);
  wait(0.3,sec);

  //intake the next ring
  chassis.drive_distance(12);
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(180);
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 1000;
  chassis.drive_distance(30);
  wait(0.5,sec);

  //touch the ladder
  chassis.turn_max_voltage = 10;
  chassis.turn_timeout = 300;
  chassis.turn_to_angle(200);
  chassis.drive_timeout = 1500;
  chassis.drive_distance(-27);//original -32
  intak.stop();
  chassis.drive_distance(-14);
  tilt.set(false);
}// end of AWP_solo_rightside

/**************************************/
//This works for both RED and BLUE alliance left side
void AWP_solo_leftside()
{
  //setup voltage, timeout and brake mode
  chassis.drive_max_voltage = 12;
  chassis.turn_max_voltage = 6;
  chassis.drive_timeout = 980;
  chassis.turn_timeout = 550;
  frontBrake();
  Lift.setBrake(hold);

  //push the blue ring back while lifting arm
  Lift.spin(fwd,100,velocityUnits::pct);
  chassis.drive_distance(-30);
  Lift.stop();
 
  //go intake the first ring
  chassis.turn_to_angle(320);
  intak.spin(fwd,10,voltageUnits::volt);
  chassis.drive_max_voltage = 5;
  chassis.drive_distance(23); //intake blue ring
  wait(0.2,sec);
  chassis.drive_max_voltage = 12;
  chassis.drive_distance(15);
  wait(0.3,sec);
  intak.stop();

  //go put the preload onto alliance stake
  chassis.drive_timeout = 700;
  chassis.drive_distance(-23);//change
  chassis.turn_timeout = 1200;
  chassis.turn_to_angle(90);//turn to face the alliance stake
  chassis.drive_timeout = 500;
  chassis.drive_distance(12);
  Lift.setTimeout(2,sec);
  Lift.spin(reverse,12,voltageUnits::volt);
  wait(0.2,sec);
  chassis.drive_timeout = 200;
  chassis.drive_distance(-3.0);
  wait(0.3,sec);
  Lift.spin(fwd,10,voltageUnits::volt);
  wait(0.1,sec);
  Lift.stop();

  //go grab the mobile goal
  chassis.turn_timeout = 500;
  chassis.drive_timeout = 500;
  chassis.drive_distance(-3.0);
  chassis.turn_to_angle(56);
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 800;
  chassis.drive_distance(-31);
  chassis.drive_max_voltage = 6;
  chassis.drive_timeout = 500;
  chassis.drive_distance(-6);
  tilt.set(true);
  wait(0.3,sec);

  //intake the next ring
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(182);
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 2000;
  chassis.drive_distance(35);
  //wait(0.5,sec);

  //touch the ladder
  chassis.turn_max_voltage = 10;
  chassis.turn_timeout = 300;
  chassis.turn_to_angle(160);
  chassis.drive_timeout = 1500;
  chassis.drive_distance(-32);
  intak.stop();
  chassis.drive_distance(-15);
   tilt.set(false);
}//end of AWP_soloL 

/***************left_+3rings*******************/
void four_rings()
{
  if(current_color_selection==0)//red
  {
  //go get the goal 
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 800;
  chassis.drive_distance(-18);
  chassis.drive_max_voltage = 6;
  chassis.drive_distance(-6);
  tilt.set(true); //clamp on the goal
  wait(0.3,sec); 

  //score the preload onto the goal
  intak.spin(fwd,12,voltageUnits::volt);

  //go get the second ring and score onto the goal
  chassis.turn_max_voltage = 6;
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(73);
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 1000;
  chassis.drive_distance(18);
  wait(1,sec);

  //go get the third ring(on the right) and score onto the goal
  chassis.drive_timeout = 500;
  chassis.drive_distance(-6);
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(132);
  chassis.drive_timeout = 800;
  chassis.drive_distance(13);
  wait(1,sec);

  //go get the 4th ring(on the left) and score onto the goal  
  chassis.drive_distance(-20);
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(100);
  chassis.drive_timeout = 800;
  chassis.drive_distance(15);
  chassis.turn_to_angle(130);
  chassis.drive_distance(10);
  wait(1,sec);
  chassis.drive_distance(-10);
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(50);
  chassis.drive_distance(-42);
  }
  else
  {
  //go get the goal 
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 800;
  chassis.drive_distance(-18);
  chassis.drive_max_voltage = 6;
  chassis.drive_distance(-6);
  tilt.set(true); //clamp on the goal
  wait(0.3,sec); 

  //score the preload onto the goal
  intak.spin(fwd,12,voltageUnits::volt);

  //go get the second ring and score onto the goal
  chassis.turn_max_voltage = 6;
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(287);
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 1000;
  chassis.drive_distance(18);
  wait(1,sec);

  //go get the third ring(on the right) and score onto the goal
  chassis.drive_timeout = 500;
  chassis.drive_distance(-6);
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(228);
  chassis.drive_timeout = 800;
  chassis.drive_distance(13);
  wait(1,sec);

  //go get the 4th ring(on the left) and score onto the goal  
  chassis.drive_distance(-20);
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(260);
  chassis.drive_timeout = 800;
  chassis.drive_distance(15);
  chassis.turn_to_angle(230);
  chassis.drive_distance(10);
  wait(1,sec);
  chassis.drive_distance(-10);
  chassis.turn_to_angle(310);
  chassis.drive_distance(-42);
  }
}// end of left_4rings*******************/


void ringDetected()
{
  color detectedColor = ringColorSensor.color();
  
  if(current_color_selection==0) //red alliance
  {//if blue ring is detected and in position
    cout<<"found ring\n";
   //if (RingDistance.objectDistance(mm) < 37 && detectedColor == blue)
    if( detectedColor == blue)
    {cout<<"found blue ring\n";
      
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
}// end of ringDectected()


/**********auton skills program***********/
void skills()
{
  //distance between clamp and wall stake is 3/8 spacer
  //BETTER TO BE CLOSER THAN FARTHER FROM THE WALL STAKE
  //ring/intake tooth at the top of the intake flex wheels
  ringColorSensor.objectDetected(ringDetected);   
  //spin preload onto alliance stake
  intak.spin(fwd,12,voltageUnits::volt);
  wait(0.5,sec);
  intak.stop();

  //get mobile goal
  chassis.drive_distance(12.5, 0, 12, 6, 1, 300, 300);
  chassis.turn_timeout = 700;
  chassis.turn_to_angle(270);
  chassis.drive_distance(-10, 270, 12, 6, 1, 300, 500);
  chassis.drive_distance(-12, 270, 6, 6, 1, 300, 1000);
  tilt.set(true);
  wait(0.3,sec);
  
  //get the other disks, starting with top right disk
  //go to grab first ring
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.turn_to_angle(0);
  chassis.drive_distance(23, 0, 8, 6, 1, 300, 1500);
  wait(0.5,sec);
  chassis.turn_timeout = 800;
 
  //second ring
  chassis.turn_to_angle(90);
  chassis.drive_distance(30);
  chassis.drive_distance(-7);

  //next two disks(3d, 4th)
  chassis.turn_to_angle(180);
  chassis.drive_max_voltage = 8;
  chassis.drive_timeout = 800;
  chassis.drive_distance(23); // og 24
  wait(0.5,sec);
  chassis.drive_timeout = 600;
  chassis.drive_distance(12); // og 14
  wait(1.0,sec);

  //last disk
  chassis.turn_timeout = 1000;
  chassis.turn_to_angle(42);
  chassis.drive_timeout = 1000;
  chassis.drive_distance(15);
  wait(1,sec);

  //put goal in the corner
  chassis.turn_timeout = 500;
  chassis.turn_to_angle(342);
  chassis.drive_timeout = 1000;
  chassis.drive_distance(-15);
  intak.stop();
  tilt.set(false);
  chassis.drive_distance(8); //move out of the corner
  chassis.turn_timeout = 1000;
  chassis.turn_to_angle(90);

  //get the goal on the left side
  chassis.drive_max_voltage = 12;
  chassis.drive_timeout = 7000;
  chassis.drive_distance(-70, 90, 12, 6, 1, 300, 3000);
  chassis.drive_max_voltage = 6;
  chassis.drive_timeout = 1500;
  chassis.drive_distance(-15, 90, 6, 6, 1, 300, 1200);
  tilt.set(true);
  wait(0.5,sec);
 
  //intake the 1st ring
  chassis.drive_max_voltage = 8;
  chassis.turn_timeout = 1000;
  chassis.turn_to_angle(0);
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_distance(24);
  wait(0.5,sec);
  chassis.turn_to_angle(270);
  
  //intake the 2nd ring
  chassis.drive_distance(26);
  wait(1,sec);
  chassis.drive_distance(-2);
  chassis.turn_to_angle(180);
  //intake the next 2 rings
  chassis.drive_distance(21);
  wait(0.5,sec);
  chassis.drive_distance(12);
  //last ring
  chassis.turn_to_angle(317);
  chassis.drive_timeout = 500;
  chassis.drive_distance(14);
  wait(0.5,sec);

  //put goal in the corner
  chassis.turn_timeout = 800;
  chassis.turn_to_angle(22);
  chassis.drive_distance(-11, 22, 12, 6, 1, 300, 800);
  tilt.set(false);
  wait(0.3,sec);

  chassis.drive_distance(36, 22, 12, 6, 1, 300, 1500);
  //temporary constants
  chassis.turn_timeout = 1000;
  chassis.drive_timeout = 2000;
  //4 above upper edge, 15.5 side wall
  //start to go to the other side
  //chassis.drive_distance(28);
  chassis.turn_to_angle(0);
  //intake first ring
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_distance(58, 0, 12, 6, 1, 300, 1500);
  intak.stop();
  chassis.drive_distance(-8, 0, 12, 6, 1, 300, 800);
  //get the mobile goal with blue ring on it
  chassis.turn_to_angle(221);
  chassis.drive_distance(-27, 221, 12, 6, 1, 300, 750);
  chassis.drive_distance(-13, 221, 6, 6, 1, 300, 650);
  tilt.set(true);
  wait(0.5,sec);

  chassis.turn_timeout = 700;
  chassis.turn_to_angle(270);
  intak.spin(fwd,12,voltageUnits::volt);
  chassis.drive_distance(30, 270, 8, 6, 1, 300, 1300);
  chassis.turn_timeout = 1000;
  chassis.turn_to_angle(132);//127
  tilt.set(false); //release goal
  wait(0.3,sec);
  chassis.drive_distance(-5, 132, 12, 6, 1, 300, 500);

//re-push
//  chassis.drive_distance(5, 120, 12, 6, 1, 300, 300);
//  chassis.drive_distance(-5, 127, 8, 6, 1, 300, 500); //127

  //get the 2nd mobile goal 
  chassis.drive_distance(21, 125, 12, 6, 1, 300, 1100);
  chassis.turn_timeout = 900;
  chassis.turn_to_angle(270);
  chassis.drive_distance(-25, 270, 12, 6, 1, 300, 1300);
  chassis.drive_distance(-9, 270, 6, 6, 1, 300, 1000);
  intak.stop();  

  tilt.set(true);
  //get the next rings
  intak.spin(fwd,12,voltageUnits::volt);
  wait(0.3,sec);
  chassis.turn_to_angle(135);
  chassis.drive_distance(39);
  chassis.turn_to_angle(90);
  chassis.drive_distance(32);

  chassis.turn_timeout = 500;
  chassis.turn_to_angle(15);
  //put the last mobile goal in the corner
  chassis.drive_distance(30);
  chassis.turn_timeout = 1500;
  chassis.turn_to_angle(230);
  intak.stop();
  chassis.drive_distance(-5);
  tilt.set(false);
  //re-push
  chassis.drive_distance(5);
  chassis.drive_distance(-5);
  //elevate
  chassis.drive_distance(10);

}//end of skills

void driveTest()
{
  //float y = backDistance.objectDistance(inches) ;
  //float x = sideDistance.objectDistance(inches) ;

  ringColorSensor.setLight(ledState::on); //turn on the optical sensor light
  ringColorSensor.setLightPower(50, percent);
  tilt.set(true);
  wait(0.8,sec);
  intak.spin(fwd,100,velocityUnits::pct); 
  wait(0.5,sec);
  ringColorSensor.objectDetected(ringDetected);
  chassis.drive_max_voltage = 4;
  chassis.drive_timeout = 5000;
  chassis.drive_distance(80);
 // cout<<"x="<<x<<" y="<<y<<"\n";
  
 /*   chassis.drive_max_voltage = 10;
  chassis.drive_timeout = 300;
  chassis.drive_distance(-5);*/

 /* chassis.drive_max_voltage = 6;
  chassis.drive_timeout = 800;
  chassis.drive_distance(-15);*/
 // tilt.set(true);
}


void autonomous(void) {
  auto_started = true;
  if (auto_started)
  {
    Brain.Screen.printAt(50, 50, "AWP solo");
    //skills(); //slot 1
    //AWP_solo_leftside(); //slot 2
    //AWP_solo_rightside(); //slot 3
    four_rings();//slot 4
    
    //driveTest();//slot 6
    
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
bool loading = 0; //not load the ring box
void loadBox()
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
}//loadBox

bool intakeRunning=false;

/*********toggle ringColorSorting options*********/
void toggle_colorSorting()
{
  if (ringColorSorting) {ringColorSorting = false;}
  else { ringColorSorting = true; }
}// end of toggle_colorSorting()

/*********use optical and distance sensors to sort rings*************/
// when the optical sensor detects rings with !current_color_section
// intake stops for 0.4 sec so the ring will go out
// otherwise intake will keep running and put the correct color ring into the goal
void sortingIntake()
{
  ringColorSensor.setLight(ledState::on); //turn on the optical sensor light
  ringColorSensor.setLightPower(50, percent);
  color detectedColor = ringColorSensor.color();
  intak.spin(fwd,100,velocityUnits::pct);

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
      ringColorSensor.setLight(ledState::off);
    }
  }
  
} //end of sortingIntake

void usercontrol(void) {
  // User control code here, inside the loop
  while (1) {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.

    // ........................................................................
    // Insert user code here. This is where you use the joystick values to
    // update your motors, etc.
    // ........................................................................
   
    if(Controller1.ButtonR1.pressing())
    {
      if (ringColorSorting) {sortingIntake();}  //do ring color sorting
      else {intak.spin(fwd,100,velocityUnits::pct);}  //not do ring color sorting
    }
    else if(Controller1.ButtonR2.pressing()){
      intak.spin(reverse,100,velocityUnits::pct);
    }
    else if(!loading)
    {
      intak.stop();
    }

    if(Controller1.ButtonL1.pressing()){  // lift going up
      Lift.setBrake(hold);
      Lift.spin(fwd,100,velocityUnits::pct);
    }
    else if(Controller1.ButtonL2.pressing()){  //lift going down
      Lift.setBrake(coast);
      Lift.spin(reverse,100,velocityUnits::pct);
    }
    else 
    {
      Lift.stop();
    } //end of if(!Controller1.ButtonR1.pressing())
    
    int a3 = Controller1.Axis3.position();
    int a2 = Controller1.Axis2.position();
    //cout<<"a1="<<a1<<" a2="<<a2<<"\n";
    if (abs(a3)>2 || abs(a2)>2) //to avoid joystick drift and random touch
    {
      int leftSpeed = a3;
      int rightSpeed = a2;
      runChassis(leftSpeed,rightSpeed);
    }
    else
    {
      Chicken.stop(brakeType::coast);
    }
    if(Controller1.ButtonRight.pressing() && Controller1.ButtonY.pressing())
    {
      elevation.set(true);
    }
    wait(20, msec); // Sleep the task for a short amount of time to
                    // prevent wasted resources.
    //cout<<"lift = "<<Lift.position(degrees)<<"\n";
  }
}

//
// Main will set up the competition functions and callbacks.
//
int main() {
  // Run the pre-autonomous function.
  pre_auton();
  
  cout<<"the code is running\n";
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);
  Controller1.ButtonA.pressed(toggleTilt);
  Controller1.ButtonY.pressed(printTemps);
  Controller1.ButtonX.pressed(loadBox);
  Controller1.ButtonUp.pressed(toggle_colorSorting);// button up pressed to turn on/off color sorting

  // Prevent main from exiting with an infinite loop.
  while (true) {
    wait(100, msec);
  }
}

