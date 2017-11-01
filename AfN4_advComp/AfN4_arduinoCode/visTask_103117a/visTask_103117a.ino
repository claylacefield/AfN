/* 
 *  
 *  Plan:
 *  - choose a trial type
 *  - send this in serial to PC/python
 *  - wait for serial from PsychoPy when 
 *    stimulus ends
 *  - start looking for choice response
 *  - give rew or pun, then after ITI,
 *    choose another trial type and send 
 *    to PsychoPy to initiate new trial
 *  
 */

// session vars (change to set session parameters)
int rewDur = 50;
int choiceDur = 5000;
int punDur = 8000;
int itiDur = 4000;
int goType = 3; // rewarded orientation (0ind of [0 45 90 135 180 225 270 315] in degrees)

int rewFreq = 2000;

// pins
int lickPin = 9;
int rewPin = 5;
int spkPin = 10;
int buttonPin = 12;

// values
int lickVal = 0;
int buttonVal = 1;
int randTrial = 0;
int freq = 1000; 
byte pyIn = 0;

// state vars
int sessionStart = 1;
int trialType = 1;
int isGoTrial = 0;
int startLickCheck = 0;

// time vars (use long)
long startLickCheckTime = 0;
long lickTime = 0;

void setup() {
  pinMode(lickPin, INPUT);
  pinMode(rewPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(9600);
  randomSeed(4);
}

void loop() {
//  buttonVal = digitalRead(buttonPin);
//  
//  if (buttonVal == LOW) {
//    if (sessionStart == 0) {
//      sessionStart = 1;
//    }
//    else {
//      sessionStart == 0;
//    }
//  }

  if (sessionStart == 1) {
    // runTrials();
    trialType = random(0,7);

    // send trialType to PsychoPy/PC
    Serial.print("trialType=");
    Serial.print(trialType);
    Serial.print(", millis=");
    Serial.println(trialType);
    
    if (trialType == goType) {
      isGoTrial = 1;
    }
    else {
      isGoTrial = 0;
    }

    // wait for serial from PsychoPy at end of vis stim
    while (startLickCheck == 0) {
      if (Serial.available()) {
        pyIn = Serial.read();
        startLickCheck = 1;
        startLickCheckTime = millis();
      }
    }

    // check for licks during choice epoch
    while (millis()-startLickCheckTime < choiceDur) {
      if (digitalRead(lickPin)) { // if animal licks spout
        lickTime = millis();
        if (isGoTrial == 1) {
          giveReward();
        }
        else {
          givePun();
        }
      } // end IF licked
    } // end WHILE for choiceDur

    startLickCheck = 0;
    delay(itiDur);
    
  } // end IF sessionStart
  
} // end loop()


// reward function
void giveReward() {
  tone(spkPin, rewFreq, 1000); // give rew tone cue
  digitalWrite(rewPin, HIGH);
  delay(rewDur);
  digitalWrite(rewPin, LOW);
  Serial.print("REWARD, millis=");
  Serial.println(lickTime);
}


// punishment function
void givePun() {
  Serial.print("PUNISHMENT, millis=");
  Serial.println(lickTime);
  while (millis()-lickTime < punDur) {  // give pun white noise stim
    freq = random(1000,4000);
    tone(spkPin, freq);
  }
  noTone(spkPin);
}

