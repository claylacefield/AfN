

int outDur = 1000;

int outPin = 13;
byte pyIn = 0;
long outStartTime = 0;
int sendOut = 0;

void setup() {
  pinMode(outPin, OUTPUT);
  Serial.begin(9600);

}

void loop() {
  if (Serial.available()) {
    pyIn = Serial.read();
    if (pyIn != 0) {
      digitalWrite(outPin, HIGH);
      outStartTime = millis();
      sendOut = 1;
    }
    else {
      digitalWrite(outPin, LOW);
      pyIn = 0;
    }

//    digitalWrite(outPin, HIGH);
//    delay(1000);
//    digitalWrite(outPin, LOW);

  }

  if (sendOut == 1 && millis()-outStartTime > outDur) {
      digitalWrite(outPin, LOW);
      sendOut = 0;
  }
}
