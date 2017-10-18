// white noise

int spkPin = 11; 
int buttonPin = 12;

int freq = 1000;


void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  
  while (digitalRead(buttonPin)==LOW) {
  freq = random(1000, 3000);
  tone(spkPin, freq);
  }

  noTone(spkPin);

}
