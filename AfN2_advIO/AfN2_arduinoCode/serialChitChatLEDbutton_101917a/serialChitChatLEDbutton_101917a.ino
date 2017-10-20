/* serialChitChat_101917
 *  Clay 2017
 *  This script does three things
 *  1.) sets up a hardware serial connection to a PC
 *      and a software serial connection to another arduino
 *  2.) mirrors the inputs from the serial monitor to the outgoing
 *      softSerial port
 *  3.) searches for a string in the input and does something
 *      if it sees a keyword
 *  
 *  
 */



#include <SoftwareSerial.h>

SoftwareSerial ardSerial(8,9); // instantiate softSer and specify RX/TX pins

String str = "";
String keyword = "dark";
int ind = -1;

int buttonPin = 12;
int ledPin = 13;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  ardSerial.begin(9600);

  str.reserve(200); // reserve 200bytes for message

  Serial.print("Starting conversation, millis=");
  Serial.println(millis());
}

void loop() {
  checkSerial();

  checkArdSerial();

  if (digitalRead(buttonPin)==LOW) {
    Serial.print("Sent: ");
    Serial.println("Let me shine some light on the subject.");
    ardSerial.println("Let me shine some light on the subject.");
    delay(1000);
  }

}

// check serial input from serial monitor/PC
void checkSerial() {

  if(Serial.available() > 0) { // if input present in buffer
     str = Serial.readStringUntil('\n');  // read until newline
     Serial.print("Sent: ");
     Serial.println(str); // echo to serial monitor display
     ardSerial.println(str);  // print this to softSer output
     //parseString();
  }
}

// check serial input from friend's arduino via softSer
void checkArdSerial() {

  if(ardSerial.available() > 0) {
     str = ardSerial.readStringUntil('\n');
     Serial.print("Received: ");
     Serial.println(str);

     parseString();
  }
}

void parseString() {

    // if partner says keywork "dark" send specific message
    if (str.indexOf("dark")>=0) {
    Serial.println("Oh, let me turn the light on for you.");
    ardSerial.println("Oh, let me turn the light on for you.");
    }
    else if (str.indexOf("light")>=0) {  // turn on led if friend says the word 'light'
      Serial.print("Sent: ");
      Serial.println("Thanks a lot!");
      ardSerial.println("Thanks a lot!");
      digitalWrite(ledPin, HIGH);
      delay(5000);
      digitalWrite(ledPin, LOW);
    }

//    if(str.substring(0)=="hello"){  // not working for some reason
//      Serial.println("hows it going?");
//    }

//// example of parsing string and extracting numerical data
//    millisInd = str.indexOf("millis="); // index of "millis" in message
//    tStr = str.substring(millisInd+8); take substring of end of "millis=" to end
//    t = tStr.toInt();  // convert to number(int)
}

