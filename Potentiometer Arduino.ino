/*
  Multi-Room Communication System
  By: Rimsha Rizvi (rrizvi3), Avi Bhatnagar (abhatn7), Shehriar Burney (sburne4)
  Description: This is a project consisting of 3 arduinos which will communicate to each other. All three arduino's have different inputs which they can use to communicate information between the arduinos.
  One arduino consists of a photoresistor which can output the amount of light present, a temperature sensor which can measure the temperature, and a potentiometer which can be used to switch between output modes.
  Each arduino also has a button which can be pressed to move onto transmit mode, and pressed again to go back to receive mode.
*/

// Including libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>

// Radio pins for CE and CSN.
RF24 radio(7, 8);

// Pin Numbers for LCD
const int rs = 9, en = A5, d4 = 6, d5 = 5, d6 = 4, d7 = A4;

// Address for radio communication
const byte address[6] = "00001";

// Pin numbers
int buttonPin = 2;
const int ledPin = A1;

// Button and debounce variables.
int buttonState = 0;
int lastButtonState = 0;
unsigned long lastDebounceTime = 0;
long unsigned int lastPress;
unsigned long debounceDelay = 500;

volatile int buttonFlag;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


unsigned long lastDataReceivedTime = 0;

const unsigned long dataTimeout = 1000;

// Struct made to transmit variables across arduinos.
struct Data {
  char ID[3] = "3:";
  char message[11] = "";
  char mode[10] = "";
};

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  pinMode(buttonPin, INPUT);

  radio.begin();
  
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);

  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.startListening();

  lcd.begin(16, 2);
}

void loop() {
  
  // When button is pressed, go to debounce function.
  if(digitalRead(buttonPin)){
    debounce();    
  }
  
  // Arduino in transmit mode.
  if (buttonState) {
    radio.stopListening();
    digitalWrite(ledPin, HIGH); // Turn on LED when the button is pressed
    
    Data dataToSend; // Variable of struct Data type

    int sensorValue = analogRead(A0); // A0 is Potentiometer pin
    int sensorMapped = map(sensorValue, 0, 1023, 1, 12);

    // Assigning different modes depending on potentiometer pin.
    if(sensorMapped <= 4){
      strcpy(dataToSend.mode, "a");
    }
    else if(sensorMapped > 4 && sensorMapped <= 8){
      strcpy(dataToSend.mode, "b");
    }
    else{
      strcpy(dataToSend.mode, "c");
    }
    
    // Write the struct variable onto radio.
    radio.write(&dataToSend, sizeof(dataToSend));
    delay(100);
  }
  // Arduino in receive mode.
  else {
    radio.startListening();
    digitalWrite(ledPin, LOW); // Turn LED off.

    // Checking to see if there is data available in the radio.
    if (radio.available()) {
      
      lastDataReceivedTime = millis();

      Data receivedData;
      radio.read(&receivedData, sizeof(receivedData));

      // This is the input received from Shehriar's arduino.
      if(strcmp(receivedData.ID, "1:") == 0){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(receivedData.ID);        
        lcd.setCursor(0, 1);
        lcd.print(receivedData.message);
      }

      // Rimsha's arduino.
      else if(strcmp(receivedData.ID, "2:") == 0){
        lcd.setCursor(0, 0);
        lcd.print(receivedData.ID);        
        lcd.setCursor(0, 1);
        lcd.print(receivedData.message);
      }
    }

    else if (millis() - lastDataReceivedTime > dataTimeout) {
      lcd.clear(); // Clear the LCD
      lastDataReceivedTime = millis();
    }

    delay(100);
  }
}

// Function used to debounce the button and make sure that the button is working properly.
void debounce() {
  if ((millis() - lastPress) > debounceDelay) {
    lastPress = millis();
    if (lastButtonState == 0) {
      Serial.println("button on");
      buttonState = 1;
    }
    else {
      Serial.println("button off");
      buttonState = 0;
    }
    lastButtonState = buttonState;
    //buttonFlag = 0;
  }
}