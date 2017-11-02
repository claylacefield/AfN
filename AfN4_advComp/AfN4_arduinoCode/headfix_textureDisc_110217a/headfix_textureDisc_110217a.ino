

String programName = "headfix_poleAngleDisc_122015a.pde";
String notes = "new pole angle discrimination task";

///// MOST IMPORTANT
int rewSolDur = 160; // time (in ms) that reward valve is open (80ms for 2p)
int rewSolDur2 = 10;
int contigTrial = 3; // max number of contiguous trials of same type (fixed 091311 so that number is actual, not n-1)
int rewStimFreq = 60; // percent of rewarded trials
int catchFreq = 0;  // percent of catch trials
int cueToneOn = 0; // specify cue tone on (1) or off (0)

int lickChoice = 1; // 1 for lickChoice, anything else for lever choice

// TIMING VARIABLES
int iti = 4000;  // 500 inter-trial interval (time after trial when no new trial can occur) (400 for lever hold iti, 2000 for timed trials)
int levPressThresh = 400;// usu 400ms or 50ms for untrained animals (how long lever has to be pressed to trigger trial)
int choiceLevThresh = 50; // how long lever has to be lifted to signal a choice (usu. 100ms)
int choiceLevThresh2 = 50;   // for unrewarded stim lever press

int stimDur = 4000;  // duration of whisker stimulus (fast = 1500)
int drinkDur = 2000; // time given for animal to drink reward water before stimulus ends (fast=1500)

int nostimTO = 6000; // nostim lever press punishment timeout
int unrewTO = 2000;  // unrewarded stim lever press punishment timeout (8000)

int cueToneDur = 1000; 
int cueToneFreq = 1000;  // frequency of reward cue tone with Tone library
int rewToneDur = 1000; 
int rewToneFreq = 2000;  // frequency of reward tone with Tone library


///// PIN MAPPING

// Solenoid and Speaker pins (Outputs)
int rewPin = 5;      //  define pin for turing solenoid valve ON
int startPin = 12;                // LED connected to digital pin 13
int punPin = 3;      // define pin for turning vacuum line ON
//int airpuffPin = 4;      // define pin for turning vacuum line OFF
int speakerOut = 9;    // define PWM pin for speaker/tone output

// Stepper pins
int enable1 = 7;
int motorPin1 = 8;  // stepper control pins
int motorPin2 = 6;    
//int motorPin3 = 10; 
//int motorPin4 = 11; 
int hallSensPin = 3;  // analog pin for Hall effect sensor (for stim arm placement)

int leverPin = 2;    // define pin for mouse lever
int leverPin2 = 13;    // define pin for exp lever

int lickPin = 12;

int linServoPin = 10;

////////////LIBRARIES
// Initialize Truerandom random number generator library
//#include <TrueRandom.h>

///// STEPPER library info
#include <Stepper.h> 
#define motorSteps 200  // 200steps/rot with Pololu steppers
Stepper stimStepper(motorSteps, motorPin1, motorPin2); //, motorPin3, motorPin4);


// stepper variables
int delayTime = 5;
int stepCW = -20;  // step size (50 = 1/4 rotation of arm)
int stepCCW = 20;

int hallSensVal = 500;   
int hallThresh = 600;

int oldPos = 0;
int newPos = 0;

//int posArr[4] = {0, 1, 2, 3};
//int rewPos1 = 0;
//int rewPos2 = 2;
int rewPos = 0;

int rotateStep = 0;
int singleStep = 1;

String oldPosString = "oldPos = ";
String newPosString = " newPos = ";

///// Linear actuator/servo
#include <Servo.h>
Servo linServo;

int linServoPos1 = 40;  // near position
int linServoPos2 = 140;  // far position

///// Lever/lick variables

int levState  = 0;
int levState2  = 0;
int prevLevState = 0;
int prevLevState2 = 0;

//int startLevLiftCheck = 0;

int lickState = 0;

///////////// OTHER VARIABLES

// time variables
long startTime = 0; 
long trigTime1 = 0;  // initialize times of beam breaks
long trigTime2 = 0;  // and times of reward
long endTime = 0;

// other time counter variables
long currTime = 0;
long elapTime = 0;  // variable for how long lever has been pressed (for triggering stimulus) or released (for triggering reward)
long elapTime2 = 0;
long elapTime3 = 0;  // variable for how long to play punishment white noise

long levPressTime = 0;
long levPressDur = 0;

long del = 0;    // initialize variables for making white noise stimulus (now presented during timeouts) 3/4/10                    
long del2 = 0;

// trial type alternation variables
int stimType = 1;
int prevType = 1;
int stimCount = 0;

// RANDOM NUMBERS
int randNumber;    // random number for whisker stimulus
int rand1 = 0;
int rand2 = 0;
int rand3 = 0;

int choice = 0;
int rew = 0;
int startSession = 1;

//SETUP////////////////////////////////
void setup()                   
{
  //pinMode(ledPin, OUTPUT);      // sets the digital pin as LED output
  pinMode(rewPin, OUTPUT);  // sets digital pin for turing solenoid valve ON
  pinMode(startPin, OUTPUT);  // sets digital pin for turing solenoid valve ON
  pinMode(punPin, OUTPUT); 
  pinMode(lickPin, INPUT);
  pinMode(leverPin, INPUT_PULLUP);
  pinMode(leverPin2, INPUT_PULLUP);

//  cueTones[0].begin(speakerOut);
  pinMode(speakerOut, OUTPUT); // not sure if I have to declare this pin
  
  // stepper setup
  stimStepper.setSpeed(60);  
  pinMode(enable1, OUTPUT);  // ENABLE pin for motor 1 (had forgotten these before)
  digitalWrite(enable1, LOW);
  // linear servo setup
  linServo.attach(linServoPin);
  linServo.write(linServoPos1);
  delay(5000);
  

  Serial.begin(9600);    // initialize serial for output to Processing sketch
  randomSeed(analogRead(3));
  Serial.println("Program name:");
  Serial.println(programName);
  Serial.println("Notes:");
  Serial.println(notes);
  Serial.println("rewarded direction = (1=top, 2=bot)");
  Serial.println(rewPos);
  Serial.println("rewarded stimulus frequency = ");
  Serial.println(rewStimFreq);
  Serial.println("contigTrial = ");
  Serial.println(contigTrial);
  Serial.println("reward amount/duration =");
  Serial.println(rewSolDur);
  Serial.println("choiceLevThresh =");
  Serial.println(choiceLevThresh);
  Serial.println("choiceLevThresh2 (catch or unrew) =");
  Serial.println(choiceLevThresh2);
  Serial.println("stimDur =");
  Serial.println(stimDur);
  Serial.println("ITI =");
  Serial.println(iti);
  Serial.println("cueToneOn =");
  Serial.println(cueToneOn);
  Serial.println("lickChoice = (1=lick, 2=lever)");
  Serial.println(lickChoice);

  Serial.println("Session START");
  Serial.println("BEGIN DATA");
}

//LOOP/////////////////////////////////
void loop()          {           // run over and over again

  

  if (startSession = 0) {
    levState2 = digitalRead(leverPin2);
    if (levState2 == HIGH) {
      Serial.println("START session");
      Serial.println(millis());
      startSession = 1;
    }
  }

  else {

    // MOVE STIM TO NEW POSITION
    // pick a number (first, last+1 indices of posArr)
    randNumber = random(0,4);
    
    // use this to find the newPos
    //newPos = posArr[randNumber];
    newPos = randNumber;
  
    // rotate stim to new location/texture
    rotateStim();
  
    // move stim forward into whisker field
    linServo.write(linServoPos2);
    
    Serial.println("trial start");
    Serial.println(millis());
    
    startTime = millis();
    elapTime = 0;
  
    digitalWrite(startPin, HIGH);  // send signal to mark start of trial
    delay(50);
    digitalWrite(startPin, LOW);
    
    while (millis()-startTime < stimDur) {  //(elapTime < stimDur) {
  
      // check for licks
      readChoice();
      //levState = digitalRead(leverPin);

      //elapTime = millis() - startTime;
      
      // if lick, is it correct or incorrect (based upon rewPos)
      if (choice == 1 || choice == 2)  {      // lickState == 1) {
      
        if (newPos == rewPos) { // || newPos == rewPos2) {
          reward();
        }
        else {
          punish();
        }
        choice = 0;
      }
      
    }

    endTime = millis();
    
    // move stim back
    linServo.write(linServoPos1);
    
  //  delay(1500);
  //
  //  // rotate stim to new location/texture
  //  rotateStim();
    
    // delay(iti);

    while (millis()-endTime < iti) {
      levState2 = digitalRead(leverPin2);
      if (levState2 == LOW) {
        Serial.println("STOP session");
        Serial.println(millis());

        newPos = 0;
        rotateStim();   // move stim to null position
        
        delay(2000);
        startSession = 0;
      }
    }
  
  }

}


// function for randomly choosing texture and rotating stim there
void rotateStim() {
  
  // NOTE: moved stim choice out of this function 122515
  
  // based upon the newPos, how far to rotate from oldPos
  rotateStep = newPos - oldPos; // *50-20; // -10;
  
  // this just stops before predicted position to let magnet do rest
  if (rotateStep <= 0) {
    rotateStep = rotateStep + 4;
    //singleStep = 1;
  }

  rotateStep = rotateStep*50-20;
  
  // now move the stim stepper to almost the final position
  digitalWrite(enable1, HIGH);
  //digitalWrite(airpuffPin, HIGH);
  delay(100);

  // now rotate stim stepper
  
  stimStepper.step(rotateStep);  // by a specific amount (for test)
  
  // and move in target direction til it reaches the magnet
  hallSensVal = analogRead(hallSensPin);
  while (hallSensVal < hallThresh) {
    stimStepper.step(singleStep);
    delay(1);  // slight delay after movement to ensure proper step before next
    hallSensVal = analogRead(hallSensPin);
    //Serial.println(hallSensVal1);
  }

  Serial.println(oldPosString + oldPos + newPosString + newPos);
  Serial.println(millis());
  
  // then turn the motor off
  delay(100);
  digitalWrite(enable1, LOW);
  
  oldPos = newPos;
}


void readChoice() {

  levState2 = digitalRead(leverPin2);

  if (levState2 == LOW) {
  //     Serial.println("experimenter lever press");
  //     Serial.printin(millis());
     choice = 2;
  }

  if (lickChoice == 1) {
    lickState = digitalRead(lickPin);
    
    if (lickState == 1) {
      choice = 1;
    }
  }
  else {  // else if lever choice
    readLeverInputs();
    
  }
}

void readLeverInputs() {

  levState = digitalRead(leverPin);

  // see how long animal lifts his hand off the lever during stim presentation
  if (levState == HIGH) {  //HIGH && prevLevState == 0) {  //|| levState2 == HIGH) {    // and if the mouse is still in the beam path, activate reward
    if (prevLevState == 0) {
      levPressTime = millis();
      prevLevState = 1;
    }
    else {
      levPressDur = millis() - levPressTime;
    }
  }
  else {
    prevLevState = 0;
    levPressDur = 0;
  }

  // trigger REWARD if animal lifts paw during rewarded stim. (for long enough)
  if (levPressDur >= choiceLevThresh) {
    choice = 1;
  }
}

// function for reward administration
void reward() {
  if (choice == 2) {
    Serial.println("experimenter REWARD");
    Serial.println(millis());
  }
  
  Serial.println("REWARD!!!");
    Serial.println(millis());
    
    // REWARD SEQUENCE
    // go through reward/vacuum solenoid sequence
    digitalWrite(rewPin, HIGH);    // open solenoid valve for a short time
    delay(rewSolDur);                  // 8ms ~= 8uL of reward liquid (on box #4 011811)
    digitalWrite(rewPin, LOW);

    // PLAY TONE
//    cueTones[0].play(rewFreq, rewToneDur);   // changing to a frequency range that's more in the region of mouse hearing
    rew = 1;
    cueTone();

    delay(drinkDur);
    
    elapTime = stimDur + 1;  // break out of the reward stimulus loop after receiving reward

}

// function for punishment administration
void punish() {

  if (choice == 2) {
    Serial.println("experimenter punishment");
    Serial.println(millis());
  }
  
  digitalWrite(punPin, HIGH);  // send signal to mark start of trial
  delay(50);
  digitalWrite(punPin, LOW);

  //digitalWrite(airpuffPin, HIGH);    // give aversive light for wrong press
  Serial.println("punishment");
  Serial.println(millis());
  delay(10);  // changed this from 2000 because air puff goes on falling phase
  //digitalWrite(airpuffPin, LOW);
    
  rew = 0;
  cueTone();

  elapTime = stimDur + 1;  // break out of stimulus loop

}

// new function for creating simple tones
// (because can't use Tone with Servo libraries)
void cueTone() {
  
  trigTime2 = millis();
  
 if (rew == 1) {  // if its a reward tone
//  while ((time-trigTime2) < rewToneDur) {
//   digitalWrite(speakerOut, HIGH);
//   delayMicroseconds(100);
//   digitalWrite(speakerOut, LOW); 
//   delayMicroseconds(150);
//   time = millis();
//  }
  tone(speakerOut, rewToneFreq, rewToneDur);
 }
 else {  // if its a punishment tone (white noise)
  while ((millis()-trigTime2) < unrewTO) {
//    rand2 = random(50,700);
//    digitalWrite(speakerOut, HIGH);
//    delayMicroseconds(rand2);
//    digitalWrite(speakerOut, LOW); 
//    delayMicroseconds(rand2);
    rand2 = random(2,10);
    tone(speakerOut, rand2*1000);
  }
  noTone(speakerOut);
 } 
    
}


