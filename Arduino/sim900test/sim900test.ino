#include <SoftwareSerial.h>

SoftwareSerial SIM900(7, 8);

#define PWRKEY_PIN 9
#define BUTTON_PIN 2

#define SIM900_BAUDRATE 9600
#define PC_BAURDRATE    9600

void setup() {
  Serial.begin(PC_BAURDRATE);
  Serial.println("Starting up...");
  
  SIM900.begin(SIM900_BAUDRATE);
  SIM900power();
  
  pinMode(BUTTON_PIN, INPUT);
  
  Serial.println("Started!");
}

void SIM900power() {
  digitalWrite(PWRKEY_PIN, HIGH);
  delay(1000);
  digitalWrite(PWRKEY_PIN, LOW);
  delay(2200);
}

int buttonState = 0;
int buttonCounter = 0;

void loop() {
  /*if (digitalRead(BUTTON_PIN) == HIGH) { // pressed
    buttonState = 1;
  } else if (buttonState == 1) { // released
    buttonState = 0;
    switch (buttonCounter) {
      case 0:
        Serial.println("Dialing...");
        SIM900.println("ATD + +972524338088;");
        break;
      case 1:
        Serial.println("Hanging up...");
        SIM900.println("ATH");
        break;
    }
    buttonCounter = (buttonCounter+1)%2;
  }*/
  
  if (SIM900.available()) {
    Serial.write(SIM900.read());
  }
  if (Serial.available()) {
    SIM900.write(Serial.read());
  }
}
