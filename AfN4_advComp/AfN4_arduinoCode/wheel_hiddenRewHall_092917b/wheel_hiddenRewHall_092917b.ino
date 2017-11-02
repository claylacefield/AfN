/*
treadmill_hiddenRewardRFID_(date).ino
Clay 2016

Serial code based on arduino SoftwareSerialExample

NOTE: 
This version is for Arduino UNO
(thus uses software serial for second serial port/rotary encoder)


111616
Added RFID code based upon Bildr tutorial.

112816
made state machine for reward solenoid
Turns on reward for lick during reward zone,
then checks after each rotary input to see if it's over.
Also records licks not in reward zone.

ToDo:
- make sure reward ends even if no rotary input
- make sure checkLicks doesn't interfere with rotary input

120116

Changing serial inputs to hardware serial.
New RFID code from
http://playground.arduino.cc/Code/ID12
Now working, after a few fixes.

ToDo:
- fix checkLicks() to eliminate multiple triggers
- format data for JSON?
- output times for lick events

120816
- track calibration
- button to start and stop

ToDo:
- fix checkLicks() to eliminate multiple triggers

121416
- reversing rotary input direction to match treadmill
- fixed multiple licks by debouncing input because it tended to spontaneously read 0 during a continuous hold

121516
- changed serial output for licks, read tag, and reward (put millis on same line)
- Reward currently only triggers if animal is moving (because it only checks zone within rotary serial input function): NEED TO FIX THIS?
- should add in non-operant reward in zone for training

121616
- made option for velocity reward (isVelRew = 1)
- changed reward solenoid pins to correct ones
- current problem: if velocity reward is on when RFID is read, it stays on: FIXED by resetting prevRew at lap too
- note that currently, reward zone will be active unless prev velocity reward since lap: FIXED by not activating reward zone for isVelRew


011717
- changed RFID code to simpler version, after correcting
  the problem with the tag string (was set to 13 bytes not 12)

091517
- adding Hall lap sensor to this code (even though there is a slightly
  newer version from March 2017 for jaeeun)

 */


/// VARIABLES TO CHANGE EXPERIMENT PARAMS
String programName = "wheel_hiddenRewHall_092917b";
int rewBegPos = 100;
int rewEndPos = 200;
int rewDur = 50;

int isButtonStart = 1;
int isOperant = 0;
int isVelRew = 1;
float velThresh = 70;
int hallThresh = 450;

int rotPosNeg = 1;  // direction of wheel rotation, 1=pos

int syncDur = 500;
int syncIntv = 5000;

// Other pins
int lickPin = 49;
int spkrPin = 47;
int ledPin = 12;
int syncPin = 13;
int trigPin = 52;
int syncPin2 = 53;

int hallPin = 0; // analog pin for hall effect lap counter

int buttonPin1 = 30;

int solPin1 = 5; //5;
int solPin2 = 4; //6;

// times
long startTime = 0;
long endTime = 0;

//
int currPos = 51;
int prevPos = 0;
int dy = 0;
float vel = 0;


// rotary encoder serial input variables
char inMess;
char outMess;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

int yInd = 0;
int mInd = 0;
String dyStr = "";
String msStr = "";
long dt = 0;
long reTime = 0;
long prevReTime = 0;

long lastHallTime = 0;

long syncStartTime = 0;
long prevSyncTime = 0;
int prevSync = 0;
long lastSyncTime = 0;

//// RFID vars
//int RFIDResetPin = 50;
//char tag1[12] = "1E009A4067A3";  // 120116: not checking tag value right now

//char tagString[12];
//int indNum = 0;
//boolean reading = false;
//int index = 0;


// initialize times and counts
int prevRew = 0;
long rewStartTime = 0;


int prevLick = 0;
long lickTime = 0;
int lickState = 0;
//int prevRew = 0;
int lickStateArr[6];
int numReading = 0;
int lickStateSum = 0;

int isCalibrated = 0;
int trackLength = 0;

long buttonTime = 0;
long prevButtonTime = 0;
int startSession = 0;

int isRewZone = 0;

// SETUP ///////////////////
void setup()
{
  // set up pins
  pinMode(lickPin, INPUT);
  //pinMode(spkrPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(solPin1, OUTPUT);
  pinMode(solPin2, OUTPUT);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(syncPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(syncPin2, OUTPUT);
  
  // Open serial communications
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println(programName);

  // set the data rate for the rotary encoder port
  // NOTE: must match the rate of the rotary encoder nano
  Serial3.begin(19200);
//  while (!Serial3) {
//    ; // wait for serial port to connect. Needed for Leonardo only
//  }
  pinMode(15, INPUT); // was having problems and a link suggested this
  digitalWrite(15, HIGH);
  //Serial3.println("Hello, world?");// reserve 200 bytes for the inputString:
  inputString.reserve(200);

//  // RFID serial port
//  Serial1.begin(9600);
//  while (!Serial1) {
//    ; // wait for serial port to connect. Needed for Leonardo only
//  }
//  pinMode(19, INPUT);
//  digitalWrite(19, HIGH);
//  
//  pinMode(RFIDResetPin, OUTPUT);
//  digitalWrite(RFIDResetPin, HIGH);

  // Now print out some header information with behavioral program parameters
  Serial.print("rewBegPos=");
  Serial.println(rewBegPos);
  Serial.print("rewEndPos=");
  Serial.println(rewEndPos);
  Serial.print("rewDur=");
  Serial.println(rewDur);
  Serial.print("isButtonStart=");
  Serial.println(isButtonStart);
  Serial.print("isOperant=");
  Serial.println(isOperant);
  Serial.print("isVelRew=");
  Serial.println(isVelRew);
  Serial.print("velThresh=");
  Serial.println(velThresh);
  Serial.println("END HEADER");
  
}  // end SETUP

// start LOOP  ///////////////////////////
void loop() // run over and over
{
    // read rotary encoder input
    serialEvent1(); //call the function
    
//    // print the string when a newline arrives:
//    if (stringComplete) {
//      //Serial.println(inputString);
//      updatePosition();  // update position and see if this is a reward zone
//      
//      // clear the string:
//      inputString = "";
//      stringComplete = false;
//    }
      
    // see if you need to turn off reward valve
    if (prevRew == 1) {
      checkRewState();
    }
  
    // read RFID input
    //serialEvent2();
    checkHall();
  
    checkLicks();

    if (isButtonStart == 1 && isCalibrated == 2) {
      checkButton();
    }

    if (startSession == 1) {
      checkSyncState();
    }

}  // end LOOP


// SUBFUNCTIONS ////////////////////

void checkSyncState() {
  if (prevSync == 1 && millis()-syncStartTime>=syncDur) {
    digitalWrite(syncPin, LOW);
    digitalWrite(syncPin2, LOW);
    prevSync = 0;
  }
  else if (millis()- lastSyncTime >= syncIntv) {
    digitalWrite(syncPin, HIGH);
    digitalWrite(syncPin2, HIGH);
    syncStartTime = millis();
    Serial.print("syncOut, millis = ");
    Serial.println(syncStartTime);
    lastSyncTime = syncStartTime;
    prevSync = 1;
  }
}


void checkButton() {
  if (digitalRead(buttonPin1) == 0) {
    if (millis() - prevButtonTime > 2000) {
    
      if (startSession == 0) {
        startSession = 1;
        startTime = millis();
        digitalWrite(trigPin, HIGH);
        Serial.print("START SESSION button, millis = ");
        Serial.println(startTime);
        Serial.print("trigTime, millis=");
        Serial.println(startTime);
        prevButtonTime = startTime;
        
      }
      else {
        startSession = 0;
        endTime = millis();
        digitalWrite(trigPin, LOW);
        digitalWrite(syncPin, LOW);
        digitalWrite(syncPin2, LOW);
        Serial.print("END session button, millis=");
        Serial.println(endTime);
        prevButtonTime = endTime;
      }
    }
  }
}

// use rotary input to update position (and check if reward zone)
void updatePosition() {

    //prevPos = currPos;
    
    yInd = inputString.indexOf("dy"); // index of "dy" in rotary encoder message
    mInd = inputString.indexOf("millis");

    dyStr = inputString.substring(yInd+4, mInd-3); 
//    Serial.println(dyStr);
    dy = dyStr.toInt();

    if (rotPosNeg == 0) {
    dy = -dy;  // 121416: reversing direction to match treadmill
    }
    msStr = inputString.substring(mInd+8, inputString.length()-1); 
    reTime = msStr.toInt();
    dt = reTime - prevReTime;
//    Serial.println(dt);
//    Serial.println(reTime);
//    Serial.println(dy);
    prevReTime = reTime;

    vel = float(dy)/float(dt)*1000;
    currPos = prevPos + dy;
    prevPos = currPos;

    // only print output and check reward zone after track calibrated and session started
    if (isCalibrated == 2 && startSession == 1) {

      Serial.print("vel=" + String(vel) + ", ");
      Serial.println("currPos=" + String(currPos) + ", millis=" + String(millis()));
  
  //    if (prevRew == 1) {
  //      checkRewState();
  //    }
  
      if (isVelRew == 0 && currPos >= rewBegPos && currPos <= rewEndPos) {
        

        isRewZone == 1;

        if (isOperant == 0 && prevRew == 0) {
          prevRew = 1;
          giveReward();
        }
        else if (digitalRead(lickPin) && prevRew == 0) {
          prevRew = 1;
          giveReward();
        }

        digitalWrite(ledPin, HIGH);
  //      delay(1000);
  //      digitalWrite(ledPin, LOW);
      }
      else if (currPos >= trackLength + 100) {
        prevPos = 0;
        prevRew = 0;  // reset reward
      }
      else {
        digitalWrite(ledPin, LOW);
        //prevRew = 0;  // reset reward if not in rew zone
        isRewZone = 0;
      }

      if (isVelRew == 1 && vel > velThresh && millis()-rewStartTime > 5000) {
        giveReward();
        prevRew = 1;
      }
      
    } // end IF isCalibrated
//    else {  // if track not calibrated, then calibrate
//      Serial.println("begin calibration");
//      
//    }
}

// reward functions
void giveReward() {
  rewStartTime = millis();
  digitalWrite(solPin1, HIGH);
  Serial.print("REWARD, millis=");
  Serial.println(rewStartTime);
}

void checkRewState() {
  if (prevRew == 1 && millis()-rewStartTime>=rewDur) {
    digitalWrite(solPin1, LOW);
    //prevRew = 0;
  }
}

void checkLicks() {
  //Serial.println(digitalRead(lickPin));
  ++numReading;
  lickState = digitalRead(lickPin);
  lickStateSum = lickStateSum + lickState;

  if (numReading == 5) {

    if (lickStateSum > 0) {
      if (prevLick == 0) { 
        prevLick = 1;
        lickTime = millis();
        Serial.print("lick, millis=");
        Serial.println(lickTime);

        if (isOperant == 1 && isRewZone == 1 && prevRew == 0) {
          prevRew = 1;
          giveReward();
        }
      
      }
    }
    else {  //if (prevLick == 1 && lickState == 0 && millis()-lickTime>50) {
      prevLick = 0;
    }
    
    numReading = 0;
    lickStateSum = 0;
  }
//  Serial.println(prevLick);
//  delay(10);
}

/* ROTARY ENCODER SERIAL INPUT
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent1() {
  while (Serial3.available()) {
    // get the new byte:
    char inChar = (char)Serial3.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      //Serial.println(inputString);
      updatePosition();  // update position and see if this is a reward zone
      
      // clear the string:
      inputString = "";
      stringComplete = false;
    }
  }
}

// Hall effect lap sensor input
void checkHall() {
  // now look for Hall effect reading
    if (analogRead(hallPin)<hallThresh && currPos > 400 && (millis()-lastHallTime)>2000) {                          // if 12 digit read is complete

      lastHallTime = millis();
      Serial.print("read tag, millis=");
      Serial.println(lastHallTime);
      //currPos=0;
      prevPos = 0;    // now just resetting position if any tag is read
      prevRew = 0;  // reset reward
      digitalWrite(solPin1, LOW);  // 121616: added this in case velocity reward ON during lap reset

      if (isCalibrated == 0) {
        isCalibrated = 1;
        Serial.println("starting track calibration");
      }
      else if (isCalibrated == 1) {
        isCalibrated = 2;
        Serial.println("calibration complete");
        trackLength = currPos;
        Serial.print("track length=");
        Serial.println(trackLength);

        if (isButtonStart==0) { 
          startSession = 1;
          startTime = millis();
          digitalWrite(trigPin, HIGH);
          Serial.print("START SESSION, millis=");
          Serial.println(startTime);
          Serial.print("trigTime, millis=");
          Serial.println(startTime);
          
          
        }  // end IF isButtonStart
      }  // end elseif isCalibrated

    }  // end IF hall is activated
}  // end VOID checkHall


