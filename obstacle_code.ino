// Pin definitions
const int trigPin = 6;
const int echoPin = 7;
const int motorPin = 10;
const int buzzerPin = 9;
const int ledPin = 5;
const int buttonPin1 = 11; // Distance mode
const int buttonPin2 = 8;  // Alert mode

// State variables
int distanceMode = 1; // 1=40cm, 2=70cm, 3=100cm
int alertMode = 1;    // 0=OFF, 1=Vib, 2=Buz, 3=Both

// Debounce
unsigned long lastPress1 = 0, lastPress2 = 0;
int lastState1 = HIGH, lastState2 = HIGH;
const unsigned long debounceTime = 200;

// Get distance
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.034 / 2;
}

// LED blink
void blinkLED(int times, int duration = 100) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW);
    delay(duration);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  Serial.println("System Ready");
}

void loop() {
  int state1 = digitalRead(buttonPin1);
  int state2 = digitalRead(buttonPin2);

  // Distance mode button
  if (state1 == LOW && lastState1 == HIGH && (millis() - lastPress1 > debounceTime)) {
    distanceMode = (distanceMode % 3) + 1;
    Serial.print("Distance Mode: ");
    Serial.println(distanceMode);
    blinkLED(distanceMode, 100);
    lastPress1 = millis();
  }
  lastState1 = state1;

  // Alert mode button
  if (state2 == LOW && lastState2 == HIGH && (millis() - lastPress2 > debounceTime)) {
    alertMode = (alertMode + 1) % 4;
    Serial.print("Alert Mode: ");
    switch (alertMode) {
      case 0: Serial.println("OFF"); break;
      case 1: Serial.println("VIBRATION"); break;
      case 2: Serial.println("BUZZER"); break;
      case 3: Serial.println("BOTH"); break;
    }
    blinkLED(2, 50);
    lastPress2 = millis();
  }
  lastState2 = state2;

  // If OFF mode, shutdown everything
  if (alertMode == 0) {
    digitalWrite(motorPin, LOW);
    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);
    return;
  }

  // Measure distance
  int distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Set distance threshold
  int threshold = 40;
  if (distanceMode == 2) threshold = 70;
  else if (distanceMode == 3) threshold = 100;

  bool inRange = (distance > 0 && distance <= threshold);

  // Activate outputs
  digitalWrite(motorPin, (alertMode == 1 || alertMode == 3) && inRange ? HIGH : LOW);

  if ((alertMode == 2 || alertMode == 3) && inRange) {
    int freq = 2000;
    if (distance > threshold * 0.8) freq = 500;
    else if (distance > threshold * 0.6) freq = 1000;
    else if (distance > threshold * 0.4) freq = 1500;
    tone(buzzerPin, freq);
  } else {
    noTone(buzzerPin);
  }

  digitalWrite(ledPin, inRange ? HIGH : LOW);

  delay(100);
}