

/* Clay 2017
 *  This sketch is for a 2-choice freely moving behavior with two stimulus sample ports.
 *  First, an animal nosepokes a sample port, at which point it is given one of two auditory (or olfactory)
 *  stimuli. For one port, tone1 indicates that one lick port is rewarded, but in the other port, tone2 indicates that 
 *  the same port is rewarded.
 *  
 *  Questions:
 *  1.) for training, make response window really large, and make either lick port rewarded
 *  2.) should ports be on same wall or adjacent
 *  3.) auditory or olfactory stimuli
 *  4.) proximal vs. distal cues
 *  5.) punishment stimulus (for animal to know it made wrong choice)
 * 
 */


//#include <Tone.h>
//Tone portTone1;
//Tone portTone2;

String programName = "jackBehav_1port_050217a.ino";

// SET VARIABLES
int stimFreq1 = 1000;
int stimFreq2 = 2000;
int stimDur = 1000;
int respWindDur = 5000;  // time after stimulus that animal must respond by licking port
int rewDur = 30; //30;
int punDur = 1000;
int analogThresh = 100;

int pokeRew = 1;

// pin #s
int portBeam1 = 0; // analog input pins for beam break
int portBeam2 = 1;

int lickPin1 = 11;  // digital pins
int lickPin2 = 12;
int spkrPin1 = 10;
int spkrPin2 = 9;
int solPin1 = 5;
int solPin2 = 6;
int buttonPin = 8;

// state variables
int portVal1 = 0;
int portVal2 = 0;

int trialType = 0;
int portNum = 0;

int prevLick = 0;
int lickPort = 0;
int prevRew = 0;

// time variables
long stimStartTime = 0;
long lickTime = 0;

/////SETUP//////////////////////////
void setup() {
  pinMode(lickPin1, INPUT);
  pinMode(lickPin2, INPUT);
  pinMode(solPin1, OUTPUT);
  pinMode(solPin2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

//  portTone1.begin(spkrPin1);
//  portTone2.begin(spkrPin2);

  Serial.begin(9600);

  randomSeed(analogRead(3));

  Serial.println(programName);
  Serial.println("stimDur=" + String(stimDur));
  Serial.println("respWindDur=" + String(respWindDur));
  Serial.println("rewDur=" + String(rewDur));
  Serial.println("punDur=" + String(punDur));
  Serial.println("stimFreq1=" + String(stimFreq1));
  Serial.println("stimFreq2=" + String(stimFreq2));
  Serial.println("analogThresh=" + String(analogThresh));
  Serial.println("pokeRew=" + String(pokeRew));
  Serial.println("END HEADER");
  Serial.println("BEGIN DATA");
  Serial.println("START SESSION");

}

/////LOOP///////////////////////////
void loop() {
  //Serial.println("portNum=" + String(portNum));
  if (portNum == 0) { // if no ports have been activated, check the ports
    //Serial.println("check ports");
    checkPorts();
  }
  else {  // if any port has been activated
    if (millis()-stimStartTime <= respWindDur) { // && prevRew == 0) {  // for duration of response window, if no previous licks
      if (prevRew == 0) {
      checkLicks(); // check for new licks
      //Serial.println("checking licks");
      }
    }
    else {  // reset trial state variables if response window elapsed
      portNum = 0;
      trialType = 0;
      prevLick = 0;
      lickPort = 0;
      prevRew = 0;
      digitalWrite(solPin1, LOW); // just make sure solenoids are closed at end of trial
      digitalWrite(solPin2, LOW); 
    }
  }
}

//////////////////////
void checkLicks() {  // during response window, check for licks

  // read lick pins
  if (digitalRead(lickPin1)) {
    lickPort = 1;
    lickTime = millis();    
  }
  else if (digitalRead(lickPin2)) {
    lickPort = 2;
    lickTime = millis();
  }

  // if any licks during response window, see if it's at the correct lickport
  if (lickPort !=0) {
    //prevLick = 1;
    if ((portNum==1 && trialType==1 && lickPort==1) || (portNum==1 && trialType==2 && lickPort==2) || (portNum==2 && trialType==1 && lickPort==2) || (portNum==2 && trialType==2 && lickPort==1)) {
      giveReward();
      Serial.println("REWARD, lickPort=" + String(lickPort) + ", millis=" + String(millis()));
      prevLick = 1;
      prevRew = 1;
      //portNum = 0;
      lickPort = 0;
    }
    else {
      Serial.println("PUN, lickPort=" + String(lickPort) + ", millis=" + String(millis()));
      givePun();
    }

    //portNum = 0;
    //trialType = 0;
    //prevLick = 0;
    //lickPort = 0;
  }
}

//////////////////////
void giveReward() {
  if (lickPort == 1) {
    digitalWrite(solPin1, HIGH);
    delay(rewDur);
    digitalWrite(solPin1, LOW);
  }
  else {
    digitalWrite(solPin2, HIGH);
    delay(rewDur);
    digitalWrite(solPin2, LOW);
  }
}

//////////////////////
void givePun() {
  delay(punDur);
}

//////////////////////
// check ports and if nosepoke, give stimulus
void checkPorts() {
  portVal1 = analogRead(portBeam1); // read both port beam break detectors
  portVal2 = analogRead(portBeam2);

  //Serial.println(portVal1);

  if (portVal1>analogThresh || portVal2>analogThresh && portNum == 0) {
    //portNum = 1;
    //Serial.println("port activated");
    stimStartTime = millis();
    
    if (portVal1>analogThresh) {  // port 1 beam break
  
      portNum = 1;
      
      if (random(0,100)>50) {
        trialType = 1;
        //portTone1.play(stimFreq1, stimDur);
        tone(spkrPin1, stimFreq1, stimDur);
        
      }
      else {
        trialType = 2;
        //portTone1.play(stimFreq2, stimDur);
        tone(spkrPin1, stimFreq2, stimDur);
      }
      
    }
    else if (portVal2>analogThresh) {
  
      portNum = 2;
      
      if (random(0,100)>50) {
        trialType = 1;
        //portTone2.play(stimFreq1, stimDur);
        tone(spkrPin2, stimFreq1, stimDur);
      }
      else {
        trialType = 2;
        //portTone2.play(stimFreq2, stimDur);
        tone(spkrPin2, stimFreq2, stimDur);
      }
      
    }
    Serial.println("TrialStart, portNum=" + String(portNum)+ ", trialType=" + String(trialType) + ", millis=" + String(millis()));  // print out port #, trial type, and time
    delay(1000);

    if (pokeRew==1) {
      rewBothPorts();
    }
    
  }
  
}

void rewBothPorts() {
    digitalWrite(solPin1, HIGH);
    digitalWrite(solPin2, HIGH);
    delay(rewDur/2);
    digitalWrite(solPin1, LOW);
    digitalWrite(solPin2, LOW);
}

