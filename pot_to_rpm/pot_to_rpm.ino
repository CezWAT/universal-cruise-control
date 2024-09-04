/*
  Based on Analog Input Arduino example
  http://www.arduino.cc/en/Tutorial/AnalogInput
  
*/

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 2 ;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH);
  delay(map(analogRead(sensorPin), 0, 1023, 40, 200));
  digitalWrite(ledPin, LOW);
  delay(map(analogRead(sensorPin), 0, 1023, 40, 200));
}
