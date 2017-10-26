int ledPin = 13;

int randInt = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  randInt = random(0,4);
  Serial.print("trialType=");
  Serial.print(String(randInt));
  Serial.print(", millis=");
  Serial.println(millis());

  delay(1000);
}
