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

// LCD pin numbers
const int rs = 9, en = A5, d4 = 6, d5 = 5, d6 = 4, d7 = A4;

const byte address[6] = "00001";

// Button variables
int buttonPin = 2;
int buttonState = 0;
int lastButtonState = 0;

const int photoResistor = A0;

unsigned long lastDebounceTime = 0;
long unsigned int lastPress;
unsigned long debounceDelay = 500;

volatile int buttonFlag;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


unsigned long lastDataReceivedTime = 0;

const unsigned long dataTimeout = 1000;

const int sampleWindow = 500;
const int sampleRate = 44000;

// Struct made to transmit variables across arduinos.
struct Data {
  char ID[3] = "1:";
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
    digitalWrite(A1, HIGH); // Turn on LED when the button is pressed
    radio.stopListening();
    
    Data dataToSend;

    // Reading values from the photoresistor and mapping it between 1-100.
    int val = analogRead(photoResistor);
    int valMap = map(val, 0, 1023, 1, 100);

    // Adding the photoresistor value to the struct as a string.
    dtostrf(valMap, 1, 0, dataToSend.message);
        
    radio.write(&dataToSend, sizeof(dataToSend));
    delay(100);
  }
  // Arduino in receive mode.  
  else {
    radio.startListening();
    digitalWrite(A1, LOW); // Turn LED off.

    // Checking to see if there is data available in the radio.
    if (radio.available()) {
      lastDataReceivedTime = millis();

      Data receivedData;
      radio.read(&receivedData, sizeof(receivedData));
      Serial.println(receivedData.mode);

      // This is the input received from Rimshas's arduino.
      if(strcmp(receivedData.ID, "2:") == 0){
        lcd.setCursor(0, 0);
        lcd.print(receivedData.ID);        
        lcd.setCursor(0, 1);
        lcd.print(receivedData.message);
      }

      // Avi's arduino.
      else if(strcmp(receivedData.ID, "3:") == 0){
        // Each mode communicates a different message on LCD.     
        if(strcmp(receivedData.mode, "a") == 0){
          lcd.clear();
          Serial.println("Yellow");
          lcd.setCursor(0, 0);
          lcd.print(receivedData.ID);        
          lcd.setCursor(0, 1);
          lcd.print("Code Yellow");    
        }
        else if(strcmp(receivedData.mode, "b") == 0){
          lcd.clear();
          Serial.println("Red");
          lcd.setCursor(0, 0);
          lcd.print(receivedData.ID);        
          lcd.setCursor(0, 1);
          lcd.print("Code Red");
        }
        else if(strcmp(receivedData.mode, "c") == 0){
          lcd.clear();
          Serial.println("Green");
          lcd.setCursor(0, 0);
          lcd.print(receivedData.ID);        
          lcd.setCursor(0, 1);
          lcd.print("Code Green");
        }
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
  }
}