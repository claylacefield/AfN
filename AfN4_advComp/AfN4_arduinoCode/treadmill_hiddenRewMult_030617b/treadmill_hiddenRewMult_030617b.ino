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

030617
Changing to Sebnem experiment parameters
- mult reward zones
- if animal licks in reward zone, can lick for 3sec for additional rewards

 */


/// VARIABLES TO CHANGE EXPERIMENT PARAMS
String programName = "treadmill_hiddenRew_011717a";
int rewBegPos = 1000;
int rewEndPos = 2000;
int rewDur = 500;

int isButtonStart = 1;  // this means that prog doesn't print out position data until button is pressed
int isOperant = 1;  // animal has to lick to get reward
int isVelRew = 0;   // reward if animal reaches a certain min velocity (in rotary clicks)
float velThresh = 400;  // for velRew
int velRewTimeout = 5000;


// mult rew locations
int numRew = 4; // number of reward zones
int rewWidth = 300;  // width of reward zone
int rewPosInds[] = {0,2,4,6}; // indices of reward zones (zero based)
int rewPosArr[] = {0, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200, 3600}; // possible start positions of reward zones
//int rewBegArr[15] = [
long rewZoneStartTime = 0;
int rewZone = 0;
int prevRewZone = 0;
int rewZoneOptTime = 3000;  // ms after entering rewZone animal has the option to lick for rew
int interLickInt = 1000;
int rewTimeout = 500;

// Other pins
int lickPin = 49;
int spkrPin = 47;
int ledPin = 13;

int buttonPin1 = 30;

int solPin1 = 5; //5;
int solPin2 = 4; //6;

// times
long startTime = 0;

//
int currPos = 0;
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
//int currPos = 0;

// RFID vars
int RFIDResetPin = 50;
//char tag1[12] = "1E009A4067A3";  // 120116: not checking tag value right now

char tagString[12];
int indNum = 0;
boolean reading = false;
int ind = 0;


// initialize times and counts
int prevRew = 0;
long rewStartTime = 0;


int prevLick = 0;
long lickTime = 0;
long prevLickTime = 0;
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
  
  // Open serial communications
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println(programName);

  // set the data rate for the rotary encoder port
  // NOTE: must match the rate of the rotary encoder nano
  Serial3.begin(19200);
  pinMode(15, INPUT); // was having problems and a link suggested this
  digitalWrite(15, HIGH);
  //Serial3.println("Hello, world?");// reserve 200 bytes for the inputString:
  inputString.reserve(200);

  // RFID serial port
  Serial1.begin(9600);
  while (!Serial1) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  pinMode(19, INPUT);
  digitalWrite(19, HIGH);
  pinMode(RFIDResetPin, OUTPUT);
  digitalWrite(RFIDResetPin, HIGH);

  // Now print out some header information with behavioral program parameters
  for (int i=0; i<numRew; i++) {
    Serial.print(rewPosArr[rewPosInds[i]]);
    Serial.print(", ");
  }
  int rewPosInds[] = {0,2,4,6}; // indices of reward zones (zero based)
  int rewPosArr[] = {0, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200, 3600}; 

//  Serial.print("rewBegPos=");
//  Serial.println(rewBegPos);
//  Serial.print("rewEndPos=");
//  Serial.println(rewEndPos);
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
      
    // see if you need to turn off reward valve (for any given reward)
    if (prevRew == 1) {
      checkRewState();
    }
  
    // read RFID input
    serialEvent2();
  
    checkLicks();

    if (isButtonStart == 1 && isCalibrated == 2) {
      checkButton();
    }

}  // end LOOP


// SUBFUNCTIONS ////////////////////



// use rotary input to update position (and check if reward zone)
void updatePosition() {

    //prevPos = currPos;
    
    yInd = inputString.indexOf("dy"); // ind of "dy" in rotary encoder message
    mInd = inputString.indexOf("millis");

    dyStr = inputString.substring(yInd+4, mInd-3); 
//    Serial.println(dyStr);
    dy = dyStr.toInt();
    dy = -dy;  // 121416: reversing direction to match treadmill
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

      // see if mouse is in any reward zone (but need to change so he can't go back through)
      for (int i=0; i<numRew; i++) { // check through all possible reward zones
        if (currPos>=rewPosArr[i] && currPos<=rewPosArr[i]+rewWidth) {  // if currPos is in a rewZone
          if (prevRewZone==0 && rewZone!=i) {  // if new entry into new zone
            rewZone = i;
            rewZoneStartTime = millis();
            Serial.println("rewZone=" + String(rewZone) + ", millis=" + String(rewZoneStartTime));
            isRewZone = 1;
            prevRewZone = 1;
          }
        }
        else {  // else if not in a reward zone
          isRewZone = 0;
          prevRewZone = 0;
          
        }
      }
  
      if (isVelRew == 0 && isRewZone == 1) {    // currPos >= rewBegPos && currPos <= rewEndPos) {
        
        //isRewZone == 1;

        if (isOperant == 0 && prevRew == 0) {
          prevRew = 1;
          giveReward();
        }
//        else if (digitalRead(lickPin) && prevRew == 0) {
//          prevRew = 1;
//          giveReward();
//        }

        digitalWrite(ledPin, HIGH);
  //      delay(1000);
  //      digitalWrite(ledPin, LOW);
      }  // end IF in rewZone
      else {
        digitalWrite(ledPin, LOW);
        //prevRew = 0;  // reset reward if not in rew zone
        isRewZone = 0;
      }

      if (currPos >= trackLength + 100) {
        prevPos = 0;
        prevRew = 0;  // reset reward
      }

      if (isVelRew == 1 && vel > velThresh && millis()-rewStartTime > velRewTimeout) {
        giveReward();
        prevRew = 1;
      }
      
    } // end IF isCalibrated
//    else {  // if track not calibrated, then calibrate
//      Serial.println("begin calibration");
//      
//    }
}

////////////////////////////////////////////
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

        if (isOperant == 1 && isRewZone == 1 && prevRew == 0 && millis()-rewZoneStartTime<=rewZoneOptTime && millis()-rewStartTime>rewTimeout) {  //  && lickTime-prevLickTime<interLickInt
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

//////////////////////////////////////
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
    prevRew = 0;
  }
}



/////////////////////////////////////
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
      
      updatePosition();  // update position and see if this is a reward zone
      
      // clear the string:
      inputString = "";
      stringComplete = false;
    }
  }
}

////////////////////////////////////////
// RFID input
void serialEvent2() {
  // now look for RFID serial
  //char tagString[12];
  //int ind = 0;
  //boolean reading = false;

  if(Serial1.available() > 0) {
    //Serial.println("Reading tag");
    while(Serial1.available()){

      int readByte = Serial1.read(); //read next available byte
  
      //Serial.println(readByte);
      
      if(readByte == 2) {
        reading = true; //begining of tag
        //Serial.println("Reading tag");
      }
      if(readByte == 3) reading = false; //end of tag
  
      // NOTE: ASCII 10 is newline and 13 is carriage return
      if(reading && readByte != 2 && readByte != 10 && readByte != 13){ // && ind <13){
        //store the tag
        tagString[ind] = readByte;
        ind ++;
      }  // end IF data byte then put in array
    } // end WHILE Serial.available

      // Output to Serial:
      //Serial.println(tagString);

    if (ind == 12) {                          // if 12 digit read is complete

      Serial.print("read tag, millis=");
      Serial.println(millis());
      //Serial.println(tagString);
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
          Serial.print("START SESSION, millis=");
          Serial.println(millis());
        }  // end IF isButtonStart
      }  // end elseif isCalibrated

      ind = 0;
    }  // end IF ind = 12

  }  // end IF Serial.available
}  // end VOID serialEvent


void checkTag(char tag[]){
///////////////////////////////////
//Check the read tag against known tags
///////////////////////////////////

  if(strlen(tag) == 0) return; //empty, no need to contunue

  //if(compareTag(tag, tag1)){ // if matched tag1, do this
    //lightLED(2);

  //}else{
    Serial.println(tag); //read out any unknown tag
//    digitalWrite(ledPin, HIGH);
//    delay(1000);
//    digitalWrite(ledPin, LOW);
    currPos = 0;
  //}

}

void clearTag(char one[]){
///////////////////////////////////
//clear the char array by filling with null - ASCII 0
//Will think same tag has been read otherwise
///////////////////////////////////
  for(int i = 0; i < strlen(one); i++){
    one[i] = 0;
  }
}

boolean compareTag(char one[], char two[]){

  if (strcmp(one, two)) {
    Serial.println("yes it's correct");
  }

  return true; //no mismatches
}

//////////////////////////////
void checkButton() {
  if (digitalRead(buttonPin1) == 0) {
    if (millis() - prevButtonTime > 2000) {
    
      if (startSession == 0) {
        startSession = 1;
        startTime = millis();
        Serial.print("START SESSION button, millis = ");
        Serial.println(startTime);
        prevButtonTime = millis();
        
      }
      else {
        startSession = 0;
        Serial.print("END session button, millis=");
        Serial.println(millis());
        prevButtonTime = millis();
      }
    }
  }
}
