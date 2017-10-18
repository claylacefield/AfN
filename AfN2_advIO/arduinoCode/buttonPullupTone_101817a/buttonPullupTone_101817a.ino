int buttonPin = 12;
int ledPin = 13;
int spkPin = 11;

int freq = 1000;
int toneDur = 1000;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  randomSeed(A4);
}

void loop() {

  if (digitalRead(buttonPin)==0) {
    freq = random(1000, 3001); // choose a new freq
    
    Serial.print("button pressed, freq=");
    Serial.print(freq);
    Serial.print(", millis=");
    Serial.println(millis());

    digitalWrite(ledPin, HIGH);
    tone(spkPin, freq, toneDur); // give tone
    //digitalWrite(ledPin, LOW);
    delay(toneDur);
    digitalWrite(ledPin, LOW);
  }

}
