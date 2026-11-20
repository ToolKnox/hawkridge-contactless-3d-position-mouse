/* This code runs the 3D echolocator robot, wich sense near environnement and can follow a moving object in orientation. If a computer is connected, the position of a detected object will control the mouse.
Attribution : I used the NewPing array code by eckel.tim http://code.google.com/p/arduino-new-ping/wiki/15_Sensors_Example I want to thank him a lot, this gave me a very good starting base.
You can download the NewPing librairy here : http://code.google.com/p/arduino-new-ping/downloads/list
*/

//Librairies
#include <Servo.h> 
#include <NewPing.h>


// Ultrasonic sensor array
#define SONAR_NUM     5 // Number or sensors.
#define MAX_DISTANCE 45 // Maximum distance (in cm) to ping. Adjust the range of objet finding (an object farther than that will not be detected).
#define PING_INTERVAL 12 // midliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).
/*
I know that setting this as low as 12 may cause cross-sensor echo, but actually it does not happen since the ping distance is also reduced. In fact, this parameter determines the speed of the robot.
The whole loop of code takes approximately 5*12 = 60 ms to execute wich means a refresh frequency of 17 Hz. Trust me, the responsiveness is barely acceptable. A higher value will give a slower response
and a lower value may lead to errors by cross-sensing. It's hard to optimise!
*/

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.


NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(8, 8, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(9, 9, MAX_DISTANCE),
  NewPing(10, 10, MAX_DISTANCE),
  NewPing(11, 11, MAX_DISTANCE),
  NewPing(12, 12, MAX_DISTANCE)
};


// Create servo objects to control the servos
Servo servo_left_down;
Servo servo_left_up;
Servo servo_right_down;
Servo servo_right_up;
Servo servo_theta; 
Servo servo_phi;

// Servo start angles (run test_servos.ino to find flat positions of sensors)
int pos_left_up = 100;
int pos_left_down = 95;
int pos_right_up = 100;
int pos_right_down = 95;
int pos_theta = 90;
int pos_phi = 40;

//Internal variables
boolean scan = false;
int iterplus = 0;
int iterminus = 0;
boolean rotation = false;
boolean left = false;
boolean right = false;
boolean up = false;
boolean down = false;
boolean mid = false;
boolean lastmid = false;

// Mouse
int clic = 0;
float xDistance = 0;
float yDistance = 0;
float zDistance = 0;


void setup() {
  Serial.begin(115200);
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  
  // Servos initialization :
  servo_theta.attach(2);
  servo_phi.attach(A0); 
  servo_left_up.attach(4);
  servo_left_down.attach(5);
  servo_right_down.attach(6);
  servo_right_up.attach(7);

  pinMode(A1, INPUT);  // Sound input pin
}

void loop() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
 
  // Write the new computed servo position
  servo_left_up.write(pos_left_up);
  servo_left_down.write(pos_left_down);
  servo_right_up.write(pos_right_up);
  servo_right_down.write(pos_right_down);
  servo_theta.write(pos_theta);
  servo_phi.write(pos_phi);
  
}

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.

// Put the sensor lengths in a more comprehensible format
  int left_up = cm[0];
  int left_down = cm[1];
  int right_down = cm[2];
  int right_up = cm[3];
  int middle = cm[4];
  
// Transform the data in booleans for a rapid treatment by the algorithm
  if ((left_up > 0)||(left_down > 0)) left = true; else left = false;
  if ((right_up > 0)||(right_down > 0)) right = true; else right = false;
  if ((left_up > 0)||(right_up > 0)) up = true; else up = false;
  if ((left_down > 0)||(right_down > 0)) down = true; else down = false;
  if (middle > 0) mid = true; else mid = false;
  
  

/* The serial monitor is handy for debugging and checking if everything is working the way it's supposed to do. The first 5 values are the sensors (returns 1 if there is an echo from an object, 0 otherwise).
You can verify the absence of cross-sensing. The 6 others (pos_something) are the angles in degrees feed to the servos. Finaly, the last (clic) is 1 if a clic is detected. You can verify that for a snap,
it's only activated once (otherwise, adjust the delay of the timer). 
*/
  Serial.print("left: ");
  Serial.print(left);
  Serial.print(" down: ");
  Serial.print(down);
  Serial.print(" right: ");
  Serial.print(right);
  Serial.print(" up: ");
  Serial.print(up);
  Serial.print(" middle: ");
  Serial.print(mid);
  Serial.print(" pos_left_up:");
  Serial.print(pos_left_up);
  Serial.print(" pos_left_down:");
  Serial.print(pos_left_down);
  Serial.print(" pos_right_up:");
  Serial.print(pos_right_up);
  Serial.print(" pos_right_down:");
  Serial.print(pos_right_down);
  Serial.print(" pos_theta:");
  Serial.print(pos_theta);
  Serial.print(" pos_phi:");
  Serial.print(pos_phi);
  Serial.print(" clic:");
  Serial.print(clic);
  Serial.println();
  
  
// Scan function : the side sensors sweep periodically in the air
    if (scan){ 
     if (left||right||up||down||mid) scan = false; // If an object is detected, turn off the scan for the next loop iteration
      if (iterplus == 12) rotation = true; // switch the sense of rotation after 12 iterations (80 degrees)
      if (iterminus == 12) rotation = false;
       if ((iterplus < 12)&&!rotation){ // Rotate towards the exterior
        pos_left_up = pos_left_up - 5;
        pos_left_down = pos_left_down + 5;
        pos_right_up = pos_right_up + 5;
        pos_right_down = pos_right_down - 5;
        ++iterplus;
        iterminus = 0;
       }
       if ((iterminus < 12)&&rotation){ // Rotate towards the interior
        pos_left_up = pos_left_up + 5;
        pos_left_down = pos_left_down - 5;
        pos_right_up = pos_right_up - 5;
        pos_right_down = pos_right_down + 5;
        ++iterminus;
        iterplus = 0;
       }  
    }
    
// When no objected is detected by any sensor, trigger the scan function at the next loop iteration    
    if (!left&&!right&&!up&&!down&&!mid&&!scan&&!lastmid) {
      pos_left_up = 90;
      pos_left_down = 97;
      pos_right_up = 100;
      pos_right_down = 95;
      scan = true;
      iterminus = 0;
      iterplus = 0;
      rotation = false;
    }
    
        
// If it's only the left sensors that detect the object, turn rapidly the robot to the left    
    if (left&&!mid&&!right) {
      pos_theta = pos_theta - 10;
      pos_left_up = pos_left_up + 7;
      pos_left_down = pos_left_down - 7;
      pos_right_up = pos_right_up - 7;
      pos_right_down = pos_right_down + 7;
    }

// If it's only the right sensors that detect the object, turn rapidly the robot to the right    
    if (!left&&!mid&&right) {
      pos_theta = pos_theta + 10;
      pos_left_up = pos_left_up + 7;
      pos_left_down = pos_left_down - 7;
      pos_right_up = pos_right_up - 7;
      pos_right_down = pos_right_down + 7;
    }
    
    

// If one side and the middle sensor detect the object, turn slowly (precise adjustment)    
    if (left&&mid&&!right) pos_theta = pos_theta - 1;
    if (!left&&mid&&right) pos_theta = pos_theta + 1;
    
// If it's only the down sensors that detects the object, turn rapidly the robot to the down    
    if (down&&!mid&&!up) {
      pos_phi = pos_phi - 10;
      pos_left_up = pos_left_up + 7;
      pos_left_down = pos_left_down - 7;
      pos_right_up = pos_right_up - 7;
      pos_right_down = pos_right_down + 7;
    }
     
// If it's only the up sensors that detects the object, turn rapidly the robot to the up    
    if (!down&&!mid&&up) {
      pos_phi = pos_phi + 10;
      pos_left_up = pos_left_up + 7;
      pos_left_down = pos_left_down - 7;
      pos_right_up = pos_right_up - 7;
      pos_right_down = pos_right_down + 7;
    }
    
// If one side and the middle sensor detect the object, turn slowly (precise adjustment)    
    if (down&&mid&&!up) pos_phi = pos_phi - 1;
    if (!down&&mid&&up) pos_phi = pos_phi + 1;
     

// If only the middle sensor detects the object, turn all the side sensors towards the interior to focus
    if (middle&&!left&&!right&&!up&&!down) {
      pos_left_up = pos_left_up + 2;
      pos_left_down = pos_left_down - 2;
      pos_right_up = pos_right_up - 2;
      pos_right_down = pos_right_down + 2;
    }

    
// If all the sensors detect the object, turn all the side sensors towards the exterior to defocus
    if (left&&mid&&right&&up&&down) {
      pos_left_up = pos_left_up - 2;
      pos_left_down = pos_left_down + 2;
      pos_right_up = pos_right_up + 2;
      pos_right_down = pos_right_down - 2;
    }
        
    lastmid = mid; // If the preceding middle sensor is not zero, do not trigger the scan (i.e. 2 consecutive zeros are necessary to initiate the scan)
    
    
// Mouse control:
  
  xDistance = (pos_theta - 90)/3; // The theta direction (horizontal angle) correspond to a displacement of cursor in the x direction
  yDistance = (40 - pos_phi)/3;  // Same thing, but for phi and y. Note the sign inversion to correspond to the reference of the mouse function. Adjust the divisor (3) for more control or more speed (you can't have both!)
  zDistance = (20-middle)/10;
  
  // Activate the mouse function only if an object is present
  if ((!scan)&&(middle != 0)) {
    Mouse.begin();
    Mouse.move(xDistance, yDistance, zDistance); // This function works with integers, the floats are rounded to the floor. It allows a certain non-response space you can adjust with divisors.
  }
  else Mouse.end();
  
  // Left clic
  clic = digitalRead(A1);
  if ((!scan)&&(middle != 0)&&clic) {
    Mouse.press(MOUSE_LEFT);
    Mouse.release(MOUSE_LEFT);
  }

  
 // Angular limits for all the servos (settled to facilitate mouse control) 
 if (pos_left_up > 100) pos_left_up = 100;
 if (pos_left_up < 30) pos_left_up = 30;
 if (pos_left_down > 150) pos_left_down = 150;
 if (pos_left_down < 80) pos_left_down = 80;
 if (pos_right_up > 150) pos_right_up = 150;
 if (pos_right_up < 80) pos_right_up = 80;
 if (pos_right_down > 100) pos_right_down = 100;
 if (pos_right_down < 30) pos_right_down = 30;
 if (pos_theta > 120) pos_theta = 120;
 if (pos_theta < 40) pos_theta = 40;
 if (pos_phi > 130) pos_phi = 130;
 if (pos_phi < 30) pos_phi = 30;

}
