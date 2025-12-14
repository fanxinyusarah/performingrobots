
/*
   By 2025 FALL Torico Chen & Sarah Fan

   Using the nRF24L01 radio module to communicate
   between two Arduinos with much increased reliability following
   various tutorials, conversations, and studying the nRF24L01 datasheet
   and the library reference.

   Transmitter is
   https://github.com/michaelshiloh/resourcesForClasses/tree/master/kicad/Arduino_Shield_RC_Controller

  Receiver is
  https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRF_servo_Mega

   This file contains code for both transmitter and receiver.
   Transmitter at the top, receiver at the bottom.
   One of them is commented out, so you need to comment in or out
   the correct section. You don't need to make changes to this 
   part of the code, just to comment in or out depending on
   whether you are programming your transmitter or receiver

   You need to set the correct address for your robot.

   Search for the phrase CHANGEHERE to see where to 
   comment or uncomment or make changes.

   These sketches require the RF24 library by TMRh20
   Documentation here: https://nrf24.github.io/RF24/index.html

   change log

   11 Oct 2023 - ms - initial entry based on
                  rf24PerformingRobotsTemplate
   26 Oct 2023 - ms - revised for new board: nRF_Servo_Mega rev 2
   28 Oct 2023 - ms - add demo of NeoMatrix, servo, and Music Maker Shield
	 20 Nov 2023 - as - fixed the bug which allowed counting beyond the limits
   22 Nov 2023 - ms - display radio custom address byte and channel
   12 Nov 2024 - ms - changed names for channel and address allocation for Fall 2024            
   31 Oct 2025 - ms - changed names for channel and address allocation for Fall 2024            
                    - listed pin numbers for servo/NeoPixel connections
                      https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRF_servo_Mega    
                      https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRFControlPanel
*/


// Common code
//

// Common pin usage
// Note there are additional pins unique to transmitter or receiver
//

// nRF24L01 uses SPI which is fixed
// on pins 11, 12, and 13 on the Uno
// and on pins 50, 51, and 52 on the Mega

// It also requires two other signals
// (CE = Chip Enable, CSN = Chip Select Not)
// Which can be any pins:

// CHANGEHERE
// For the transmitter
// const int NRF_CE_PIN = A4, NRF_CSN_PIN = A5;

// CHANGEHERE
// for the receiver
struct __attribute__((packed)) Data {
  uint8_t stateNumber;
};

const int NRF_CE_PIN = A11, NRF_CSN_PIN = A15;

// nRF 24L01 pin   name
//          1      GND
//          2      3.3V
//          3      CE
//          4      CSN
//          5      SCLK
//          6      MOSI/COPI
//          7      MISO/CIPO

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);  // CE, CSN

//#include <printf.h>  // for debugging

// See note in rf24Handshaking about address selection
//

// Channel and address allocation:
// Torico and Sarah: Channel 30, addr = 0x76
// Sudiksha and Aysha: Channel 40, addr = 0x73
// Mariam and Joy:  Channel 50, addr = 0x7C
// Ghadir and Mustafa: Channel 60, addr = 0xC6
// Clara and Jiho:  Channel 70, addr = 0xC3
// Victor and Meera: Channel 80, addr = 0xCC
// Ali and Hari: Channel 90, addr = 0x33

// CHANGEHERE
const int CUSTOM_CHANNEL_NUMBER = 30;   // change as per the above assignment
const byte CUSTOM_ADDRESS_BYTE = 0x76;  // change as per the above assignment

// Do not make changes here
const byte xmtrAddress[] = { CUSTOM_ADDRESS_BYTE, CUSTOM_ADDRESS_BYTE, 0xC7, 0xE6, 0xCC };
const byte rcvrAddress[] = { CUSTOM_ADDRESS_BYTE, CUSTOM_ADDRESS_BYTE, 0xC7, 0xE6, 0x66 };

const int RF24_POWER_LEVEL = RF24_PA_LOW;

// global variables
uint8_t pipeNum;
unsigned int totalTransmitFailures = 0;

struct DataStruct {
  uint8_t stateNumber;
};
DataStruct data;

void setupRF24Common() {

  // RF24 setup
  if (!radio.begin()) {
    Serial.println(F("radio  initialization failed"));
    while (1)
      ;
  } else {
    Serial.println(F("radio successfully initialized"));
  }

  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(CUSTOM_CHANNEL_NUMBER);
  radio.setPALevel(RF24_POWER_LEVEL);
}

// CHANGEHERE
/*// Transmitter code
struct __attribute__((packed)) Data {
  uint8_t stateNumber;
};

// Transmitter pin usage
const int LCD_RS_PIN = 3, LCD_EN_PIN = 2, LCD_D4_PIN = 4, LCD_D5_PIN = 5, LCD_D6_PIN = 6, LCD_D7_PIN = 7;
const int SW1_PIN = 8, SW2_PIN = 9, SW3_PIN = 10, SW4_PIN = A3, SW5_PIN = A2;

// LCD library code
#include <LiquidCrystal.h>

// initialize the library with the relevant pins
LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);


const int NUM_OF_STATES = 30;
char* theStates[] = { "0 robot mew",
                      "1 robot /",
                      "2 robot /",
                      "3 robot /",
                      "4 robot /",
                      "5 robot /" };

void updateLCD() {

  lcd.clear();
  lcd.print(theStates[data.stateNumber]);
  lcd.setCursor(0, 1);  // column, line (from 0)
  lcd.print("not transmitted yet");
}

void countDown() {
  data.stateNumber = (data.stateNumber > 0) ? (data.stateNumber - 1) : 0;
  updateLCD();
}

void countUp() {
  if (++data.stateNumber >= NUM_OF_STATES) {
    data.stateNumber = NUM_OF_STATES - 1;
  }
  updateLCD();
}


void spare1() {}
void spare2() {}

void rf24SendData() {

  radio.stopListening();  // go into transmit mode
  // The write() function will block
  // until the message is successfully acknowledged by the receiver
  // or the timeout/retransmit maxima are reached.
  // Returns 1 if write succeeds
  // Returns 0 if errors occurred (timeout or FAILURE_HANDLING fails)
  int retval = radio.write(&data, sizeof(data));
  
  lcd.clear();
  lcd.setCursor(0, 0);  // column, line (from 0)
  lcd.print("transmitting");
  lcd.setCursor(14, 0);  // column, line (from 0)
  lcd.print(data.stateNumber);

  Serial.print(F(" ... "));
  if (retval) {
    Serial.println(F("success"));
    lcd.setCursor(0, 1);  // column, line (from 0)
    lcd.print("success");
  } else {
    totalTransmitFailures++;
    Serial.print(F("failure, total failures = "));
    Serial.println(totalTransmitFailures);

    lcd.setCursor(0, 1);  // column, line (from 0)
    lcd.print("error, total=");
    lcd.setCursor(13, 1);  // column, line (from 0)
    lcd.print(totalTransmitFailures);
  }
}

class Button {
  int pinNumber;
  bool previousState;
  void (*buttonFunction)();
public:

  // Constructor
  Button(int pn, void* bf) {
    pinNumber = pn;
    buttonFunction = bf;
    previousState = 1;
  }

  // update the button
  void update() {
    bool currentState = digitalRead(pinNumber);
    if (currentState == LOW && previousState == HIGH) {
      Serial.print("button on pin ");
      Serial.print(pinNumber);
      Serial.println();
      buttonFunction();
    }
    previousState = currentState;
  }
};

const int NUMBUTTONS = 5;
Button theButtons[] = {
  Button(SW1_PIN, countDown),
  Button(SW2_PIN, rf24SendData),
  Button(SW3_PIN, countUp),
  Button(SW4_PIN, spare1),
  Button(SW5_PIN, spare2),
};

void setupRF24() {

  setupRF24Common();

  // Set us as a transmitter
  radio.openWritingPipe(xmtrAddress);
  radio.openReadingPipe(1, rcvrAddress);

  // radio.printPrettyDetails();
  Serial.println(F("I am a transmitter"));

  data.stateNumber = 0;
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("Setting up LCD"));

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  // Print a message to the LCD.
  lcd.print("Radio setup");

  // Display the address in hex
  lcd.setCursor(0, 1);
  lcd.print("addr 0x");
  lcd.setCursor(7, 1);
  char s[5];
  sprintf(s, "%02x", CUSTOM_ADDRESS_BYTE);
  lcd.print(s);

  // Display the channel number
  lcd.setCursor(10, 1);
  lcd.print("ch");
  lcd.setCursor(13, 1);
  lcd.print(CUSTOM_CHANNEL_NUMBER);

  Serial.println(F("Setting up radio"));
  setupRF24();

  // If setupRF24 returned then the radio is set up
  lcd.setCursor(0, 0);
  lcd.print("Radio OK state=");
  lcd.print(theStates[data.stateNumber]);

  // Initialize the switches
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);
  pinMode(SW4_PIN, INPUT_PULLUP);
  pinMode(SW5_PIN, INPUT_PULLUP);
}



void loop() {
  for (int i = 0; i < NUMBUTTONS; i++) {
    theButtons[i].update();
  }
  delay(50);  // for testing
}


void clearData() {
  // set all fields to 0
  data.stateNumber = 0;
}

// End of transmitter code
// CHANGEHERE
*/

// Receiver Code
// CHANGEHERE

// Additional libraries for music maker shield
#include <Adafruit_VS1053.h>
#include <SD.h>

// Servo library
#include <Servo.h>

// Additional libraries for graphics on the Neo Pixel Matrix
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif

// Additional pin usage for receiver

// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

// Connectors for NeoPixels and Servo Motors are labeled
// on the circuit board
// and use pins 16, 17, 18, 19, 20, and 21 

// Servo motors
// const int NOSE_SERVO_PIN = 20;
//const int ANTENNA_SERVO_PIN = 16;
//const int TAIL_SERVO_PIN = 17;
//const int GRABBER_SERVO_PIN = 18;

// Neopixel
const int NEOPIXELPIN = 19;
const int NUMPIXELS = 128;
// #define NEOPIXELPIN 19
// #define NUMPIXELS 128  // change to fit
//Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 8, NEOPIXELPIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                            NEO_GRB            + NEO_KHZ800);

#define LED_PIN 20
#define LED_COUNT 33

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

Servo LeftArm;
Servo RightArm;
Servo Head;
Servo Antenna;

int LeftArmPin = 21;  // M2
int RightArmPin = 17; // M6
int HeadPin = 16;     // M4
int AntennaPin = 18;  // 

// change as per your robot
// const int NOSE_WRINKLE = 45;
// const int NOSE_TWEAK = 90;
// const int TAIL_ANGRY = 0;
// const int TAIL_HAPPY = 180;
// const int GRABBER_RELAX = 0;
// const int GRABBER_GRAB = 180;

// LED strip flashing variable
unsigned long lastDiscoUpdate = 0;
unsigned long discoInterval = 50;  // 50ms


void setup() {
  Serial.begin(9600);
  // printf_begin();

  // Set up all the attached hardware
  setupMusicMakerShield();
  setupServoMotors();
  setupNeoPixels();

  setupRF24();

  // Brief flash to show we're done with setup()
  flashNeoPixels();
}

void setupRF24() {
  setupRF24Common();

  // Set us as a receiver
  radio.openWritingPipe(rcvrAddress);
  radio.openReadingPipe(1, xmtrAddress);

  // radio.printPrettyDetails();
  Serial.print(F("I am a receiver on channel "));
  Serial.print(CUSTOM_CHANNEL_NUMBER);
  Serial.print (" and at address 0x");
  Serial.print (CUSTOM_ADDRESS_BYTE, HEX);
  Serial.println("");
}

void setupMusicMakerShield() {
  if (!musicPlayer.begin()) {  // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1)
      ;
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD card failed or not present"));
    while (1)
      ;  // don't do anything more
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
}

void setupServoMotors() {
  LeftArm.attach(LeftArmPin);
  RightArm.attach(RightArmPin);
  Head.attach(HeadPin);
  Antenna.attach(AntennaPin);

  LeftArm.write(60);   
  RightArm.write(100); 
  Head.write(90);
  Antenna.write(90); 
  delay(1000);
}


void setupNeoPixels() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(matrix.Color(200, 30, 40));

  matrix.clear();


}

void flashNeoPixels() {

  // Using the Matrix library
  matrix.fillScreen(matrix.Color(0, 255, 0));
  matrix.show();
  delay(500);
  matrix.fillScreen(0);
  matrix.show();

  //  // all on
  //  for (int i = 0; i < NUMPIXELS; i++) {  // For each pixel...
  //    pixels.setPixelColor(i, pixels.Color(0, 100, 0));
  //  }
  //  pixels.show();
  //  delay(500);
  //
  //  // all off
  //  pixels.clear();
  //  pixels.show();
}

void loop() {
  // If there is data, read it,
  // and do the needfull
  // Become a receiver
  radio.startListening();
  if (radio.available(&pipeNum)) {
    radio.read(&data, sizeof(data));
    Serial.print(F("message received Data = "));
    Serial.print(data.stateNumber);
    Serial.println();

    switch (data.stateNumber) {
   //CASE 0   
  case 0:
  {
  Serial.println(F("Case 0"));

  matrix.clear();
  // Initial square pattern (centered 4x4)
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255, 255, 255)); 
    }
  }
for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x + 8, y, matrix.Color(255, 255, 255));
    }
  }
  matrix.show();

  //LED
  strip.begin();
  strip.show();

// white light
for (int i = 0; i < LED_COUNT; i++) {
  strip.setPixelColor(i, 50, 50, 50);  
}
strip.show();
  break;
  }
//CASE 1 
// case 1: {
//   Serial.println(F("Case 1"));
  
//  
//   lastDiscoUpdate = millis();
  
//   int leftCenter = 60;
//   int rightCenter = 100;
//   int armStep = 30;
//   int smoothSteps = 100;
//   int headLeft = 75;
//   int headRight = 105;
//   int headCenter = 90;
  
//  
//   for (int i = 0; i <= smoothSteps; i++) {
//     float t = i / (float)smoothSteps;
//     Head.write(headCenter + (headLeft - headCenter) * t);
//     LeftArm.write(leftCenter + (-armStep - leftCenter) * t);
//     RightArm.write(rightCenter + (-armStep - rightCenter) * t);
    
//     unsigned long now = millis();
//     if (now - lastDiscoUpdate > discoInterval) {
//       lastDiscoUpdate = now;
//       for (int j = 0; j < LED_COUNT; j++) {
//         strip.setPixelColor(j,
//           random(20, 60), 
//           random(20, 60),
//           random(20, 60)
//         );
//       }
//       strip.show();
//     }
    
//     delay(30);
//   }
//   delay(100);

//   for (int i = 0; i <= smoothSteps; i++) {
//     float t = i / (float)smoothSteps;
//     Head.write(headLeft + (headRight - headLeft) * t);
//     LeftArm.write((leftCenter - armStep) + (leftCenter - (leftCenter - armStep)) * t);
//     RightArm.write((rightCenter - armStep) + (rightCenter - (rightCenter - armStep)) * t);
    
//     unsigned long now = millis();
//     if (now - lastDiscoUpdate > discoInterval) {
//       lastDiscoUpdate = now;
//       for (int j = 0; j < LED_COUNT; j++) {
//         strip.setPixelColor(j,
//           random(20, 60),
//           random(20, 60),
//           random(20, 60)
//         );
//       }
//       strip.show();
//     }
    
//     delay(30);
//   }
//   delay(100);

//   for (int i = 0; i <= smoothSteps; i++) {
//     float t = i / (float)smoothSteps;
//     Head.write(headRight + (headCenter - headRight) * t);
//     LeftArm.write(leftCenter + (-armStep - leftCenter) * t);
//     RightArm.write(rightCenter + (-armStep - rightCenter) * t);
    
//     unsigned long now = millis();
//     if (now - lastDiscoUpdate > discoInterval) {
//       lastDiscoUpdate = now;
//       for (int j = 0; j < LED_COUNT; j++) {
//         strip.setPixelColor(j,
//           random(20, 60),
//           random(20, 60),
//           random(20, 60)
//         );
//       }
//       strip.show();
//     }
    
//     delay(30);
//   }
//   delay(100);
  
//   for (int i = 0; i <= smoothSteps; i++) {
//     float t = i / (float)smoothSteps;
//     Head.write(headCenter);
//     LeftArm.write((leftCenter - armStep) + (leftCenter - (leftCenter - armStep)) * t);
//     RightArm.write((rightCenter - armStep) + (rightCenter - (rightCenter - armStep)) * t);
    
//     unsigned long now = millis();
//     if (now - lastDiscoUpdate > discoInterval) {
//       lastDiscoUpdate = now;
//       for (int j = 0; j < LED_COUNT; j++) {
//         strip.setPixelColor(j,
//           random(20, 60),
//           random(20, 60),
//           random(20, 60)
//         );
//       }
//       strip.show();
//     }
    
//     delay(30);
//   }
  
//   break;
// }
case 1: {
  Serial.println(F("Case 1"));
  
  lastDiscoUpdate = millis();
  
  int leftCenter = 60;
  int rightCenter = 100;
  int armStep = 30;
  int smoothSteps = 100;
  int headLeft = 75;
  int headRight = 105;
  int headCenter = 90;
  
// 1. Head moves from left to right, while arms move forward (left front / right back).
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    Head.write(headCenter + (headLeft - headCenter) * t);
// Fix: Left front = leftCenter - armStep * t
    LeftArm.write(leftCenter - armStep * t);
// Fix: Right Back = rightCenter + armStep * t
    RightArm.write(rightCenter + armStep * t);
    
    unsigned long now = millis();
    if (now - lastDiscoUpdate > discoInterval) {
      lastDiscoUpdate = now;
      for (int j = 0; j < LED_COUNT; j++) {
        strip.setPixelColor(j,
          random(20, 60),
          random(20, 60),
          random(20, 60)
        );
      }
      strip.show();
    }
    
    delay(30);
  }
  delay(100);
  
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    Head.write(headLeft + (headRight - headLeft) * t);
    LeftArm.write((leftCenter - armStep) + armStep * t);
    RightArm.write((rightCenter + armStep) - armStep * t);
    
// Random blinking effect
    unsigned long now = millis();
    if (now - lastDiscoUpdate > discoInterval) {
      lastDiscoUpdate = now;
      for (int j = 0; j < LED_COUNT; j++) {
        strip.setPixelColor(j,
          random(20, 60),
          random(20, 60),
          random(20, 60)
        );
      }
      strip.show();
    }
    
    delay(30);
  }
  delay(100);
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    Head.write(headRight + (headCenter - headRight) * t);
    // Fix: left arm forward = leftCenter - armStep * t
    LeftArm.write(leftCenter - armStep * t);
    // Fix: right arm backward = rightCenter + armStep * t
    RightArm.write(rightCenter + armStep * t);
    
    // Random disco flicker
    unsigned long now = millis();
    if (now - lastDiscoUpdate > discoInterval) {
      lastDiscoUpdate = now;
      for (int j = 0; j < LED_COUNT; j++) {
        strip.setPixelColor(j,
          random(20, 60),
          random(20, 60),
          random(20, 60)
        );
      }
      strip.show();
    }
    
    delay(30);
  }
  delay(100);
  
  // 4. Return to center (head + both arms)
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    Head.write(headCenter);
    // Fix: from (leftCenter - armStep) back to leftCenter
    LeftArm.write((leftCenter - armStep) + armStep * t);
    // Fix: from (rightCenter + armStep) back to rightCenter
    RightArm.write((rightCenter + armStep) - armStep * t);
    
    // Random disco flicker
    unsigned long now = millis();
    if (now - lastDiscoUpdate > discoInterval) {
      lastDiscoUpdate = now;
      for (int j = 0; j < LED_COUNT; j++) {
        strip.setPixelColor(j,
          random(20, 60),
          random(20, 60),
          random(20, 60)
        );
      }
      strip.show();
    }
    
    delay(30);
  }
  
  break;
}
// CASE 2: ^^ eyes, antenna swings left and right
case 2: {
  Serial.println(F("Case 2"));
  
  // --- "^^" eyes ---
  matrix.clear();
  
  // left ^^
  matrix.drawPixel(1, 4, matrix.Color(255,255,255));
  matrix.drawPixel(2, 3, matrix.Color(255,255,255));
  matrix.drawPixel(3, 2, matrix.Color(255,255,255));
  matrix.drawPixel(4, 1, matrix.Color(255,255,255));
  matrix.drawPixel(5, 2, matrix.Color(255,255,255));
  matrix.drawPixel(6, 3, matrix.Color(255,255,255));
  matrix.drawPixel(7, 4, matrix.Color(255,255,255));
  
  // right ^^
  matrix.drawPixel(9, 4, matrix.Color(255,255,255));
  matrix.drawPixel(10, 3, matrix.Color(255,255,255));
  matrix.drawPixel(11, 2, matrix.Color(255,255,255));
  matrix.drawPixel(12, 1, matrix.Color(255,255,255));
  matrix.drawPixel(13, 2, matrix.Color(255,255,255));
  matrix.drawPixel(14, 3, matrix.Color(255,255,255));
  matrix.drawPixel(15, 4, matrix.Color(255,255,255));
  
  matrix.show();
  
  // LED strip: dim white idle light
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 50, 50, 50);
  }
  strip.show();
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/000000a2.mp3");
  }
  
  // Antenna: rotate left 45°
  for (int i = 0; i <= 45; i++) {
    Antenna.write(90 - i);
    delay(10);
  }
  
  // Rotate right 90°
  for (int i = 0; i <= 90; i++) {
    Antenna.write(45 + i);
    delay(10);
  }
  
  // Return to center 45°
  for (int i = 0; i <= 45; i++) {
    Antenna.write(135 - i);
    delay(10);
  }
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/000000b2.mp3");
  }
  
  // arms
  int leftCenter = 60;
  int rightCenter = 100;
  int armStep = 30;
  int smoothSteps = 40;
  
  // forward
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(leftCenter - armStep * t);
    RightArm.write(rightCenter + armStep * t);
    delay(20);
  }
  delay(100);
  
  // backward
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write((leftCenter - armStep) + armStep * t * 2);
    RightArm.write((rightCenter + armStep) - armStep * t * 2);
    delay(20);
  }
  delay(100);
  
  // forward
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write((leftCenter + armStep) - armStep * t * 2);
    RightArm.write((rightCenter - armStep) + armStep * t * 2);
    delay(20);
  }
  delay(100);
  
  // back to center
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write((leftCenter - armStep) + armStep * t);
    RightArm.write((rightCenter + armStep) - armStep * t);
    delay(20);
  }
  
  break;
}
// CASE 3: head turns left to right, eyes "- -"
case 3:
{
  Serial.println(F("Case 3"));
  matrix.clear();

// eyes "- -"
// left
matrix.drawPixel(1, 3, matrix.Color(255,255,255));
matrix.drawPixel(2, 3, matrix.Color(255,255,255));
matrix.drawPixel(3, 3, matrix.Color(255,255,255));
matrix.drawPixel(4, 3, matrix.Color(255,255,255));
matrix.drawPixel(5, 3, matrix.Color(255,255,255));
matrix.drawPixel(6, 3, matrix.Color(255,255,255));

matrix.drawPixel(1, 4, matrix.Color(255,255,255));
matrix.drawPixel(2, 4, matrix.Color(255,255,255));
matrix.drawPixel(3, 4, matrix.Color(255,255,255));
matrix.drawPixel(4, 4, matrix.Color(255,255,255));
matrix.drawPixel(5, 4, matrix.Color(255,255,255));
matrix.drawPixel(6, 4, matrix.Color(255,255,255));

  // right
  
// Row 3 (y = 3)
matrix.drawPixel(9, 3, matrix.Color(255,255,255));
matrix.drawPixel(10, 3, matrix.Color(255,255,255));
matrix.drawPixel(11, 3, matrix.Color(255,255,255));
matrix.drawPixel(12, 3, matrix.Color(255,255,255));
matrix.drawPixel(13, 3, matrix.Color(255,255,255));
matrix.drawPixel(14, 3, matrix.Color(255,255,255));

// Row 4 (y = 4)
matrix.drawPixel(9, 4, matrix.Color(255,255,255));
matrix.drawPixel(10, 4, matrix.Color(255,255,255));
matrix.drawPixel(11, 4, matrix.Color(255,255,255));
matrix.drawPixel(12, 4, matrix.Color(255,255,255));
matrix.drawPixel(13, 4, matrix.Color(255,255,255));
matrix.drawPixel(14, 4, matrix.Color(255,255,255));

matrix.show();

// sound
if (!musicPlayer.playingMusic) {
  musicPlayer.startPlayingFile("/00000003.mp3");
}

// head movement
for (int pos = 90; pos >= 60; pos--) {
  Head.write(pos);
  delay(20);
}
for (int pos = 70; pos <= 120; pos++) {
  Head.write(pos);
  delay(20);
}
for (int pos = 120; pos >= 90; pos--) {
  Head.write(pos);
  delay(20);
}
  break;
}
// CASE 4: eye reset / neutral
case 4:
{
  Serial.println(F("Case 4"));
  matrix.clear();
  
  // Initial 4x4 center square
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255, 255, 255)); 
    }
  }
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x + 8, y, matrix.Color(255, 255, 255));
    }
  }
  matrix.show(); 
  break;
}

  case 5:
  // CASE 5 eye- - 
  {
    Serial.println(F("Case 5"));
       matrix.clear();

// eyes"- -"
//left:
matrix.drawPixel(1, 3, matrix.Color(255,255,255));
matrix.drawPixel(2, 3, matrix.Color(255,255,255));
matrix.drawPixel(3, 3, matrix.Color(255,255,255));
matrix.drawPixel(4, 3, matrix.Color(255,255,255));
matrix.drawPixel(5, 3, matrix.Color(255,255,255));
matrix.drawPixel(6, 3, matrix.Color(255,255,255));

matrix.drawPixel(1, 4, matrix.Color(255,255,255));
matrix.drawPixel(2, 4, matrix.Color(255,255,255));
matrix.drawPixel(3, 4, matrix.Color(255,255,255));
matrix.drawPixel(4, 4, matrix.Color(255,255,255));
matrix.drawPixel(5, 4, matrix.Color(255,255,255));
matrix.drawPixel(6, 4, matrix.Color(255,255,255));

  // right 
  
// 3rd row（y = 3）
matrix.drawPixel(9, 3, matrix.Color(255,255,255));
matrix.drawPixel(10, 3, matrix.Color(255,255,255));
matrix.drawPixel(11, 3, matrix.Color(255,255,255));
matrix.drawPixel(12, 3, matrix.Color(255,255,255));
matrix.drawPixel(13, 3, matrix.Color(255,255,255));
matrix.drawPixel(14, 3, matrix.Color(255,255,255));

// 4th row（y = 4）
matrix.drawPixel(9, 4, matrix.Color(255,255,255));
matrix.drawPixel(10, 4, matrix.Color(255,255,255));
matrix.drawPixel(11, 4, matrix.Color(255,255,255));
matrix.drawPixel(12, 4, matrix.Color(255,255,255));
matrix.drawPixel(13, 4, matrix.Color(255,255,255));
matrix.drawPixel(14, 4, matrix.Color(255,255,255));


matrix.show();
 // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/0000mm05.mp3");
  }
// antenna
  for (int i = 0; i <= 90; i++) {   // left 180°
    Antenna.write(90 - i);
    delay(10);
  }
  for (int i = 0; i <= 180; i++) {  // right 180°
    Antenna.write(i);
    delay(10);
  }
  for (int i = 0; i <= 90; i++) {   // back to middle
    Antenna.write(180 - i);
    delay(10);
  }
 // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000005.mp3");
  }
    break;
  }
case 6: {
  Serial.println(F("Case 6"));
  
  // LED blink effect (multiple flashes)
  Serial.println(F("LED blinking..."));
  int blinkCount = 3;  // Number of blinks
  int blinkDelay = 200;  // Blink interval (ms)
  
  for (int blink = 0; blink < blinkCount; blink++) {
    // ON (dim red)
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 30, 0, 0);  // Dim red
    }
    strip.show();
    delay(blinkDelay);
    
    // OFF
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
    delay(blinkDelay);
  }
  
  // After blinking, keep red light on
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 30, 0, 0);  // Dim red steady
  }
  strip.show();
  Serial.println(F("Red light set - starting arm movement..."));
  
  int center = 100;
  int target = center + 30;   // Raise arm by 30°
  int smoothSteps = 40;
  
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    int armPos = center + (target - center) * t;   // 100 → 130
    RightArm.write(armPos);
    delay(20);
  }
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000006.mp3");
  }
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/000000a6.mp3");
  }
  
  break;
}

case 7: {
  Serial.println(F("Case 7"));
  
  // LED blink effect (multiple flashes)
  Serial.println(F("LED blinking..."));
  int blinkCount = 3;  // Number of blinks
  int blinkDelay = 200;  // Blink interval (ms)
  
  for (int blink = 0; blink < blinkCount; blink++) {
    // ON (brighter red than case 6)
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 60, 0, 0);  // Brighter red
    }
    strip.show();
    delay(blinkDelay);
    
    // OFF
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
    delay(blinkDelay);
  }
  
  // After blinking, keep red light on
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 60, 0, 0);  // Bright red steady
  }
  strip.show();
  Serial.println(F("Red light maintained - starting arm movement..."));
  
  // Arm continues from case 6
  int startPos = 100 + 30;    // End of case 6 = 130°
  int endPos   = 100 + 60;    // Target = 160°
  int smoothSteps = 40;
  
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    int armPos = startPos + (endPos - startPos) * t; // 130 → 160
    RightArm.write(armPos);
    delay(20);
  }
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/000000n7.mp3");
  }
  
  break;
}
// CASE 8: sad eyes (╯╰), arms reset
case 8: {
  Serial.println(F("Case 8"));
  
  matrix.clear();
  
  // Left eye ╯ (inner high, outer low)
  matrix.drawPixel(1, 6, matrix.Color(255,255,255));
  matrix.drawPixel(2, 6, matrix.Color(255,255,255));
  matrix.drawPixel(3, 5, matrix.Color(255,255,255));
  matrix.drawPixel(4, 5, matrix.Color(255,255,255));
  matrix.drawPixel(5, 4, matrix.Color(255,255,255));
  matrix.drawPixel(6, 4, matrix.Color(255,255,255));
  
  // Right eye ╰ (inner high, outer low)
  matrix.drawPixel(14, 6, matrix.Color(255,255,255));
  matrix.drawPixel(13, 6, matrix.Color(255,255,255));
  matrix.drawPixel(12, 5, matrix.Color(255,255,255));
  matrix.drawPixel(11, 5, matrix.Color(255,255,255));
  matrix.drawPixel(10, 4, matrix.Color(255,255,255));
  matrix.drawPixel(9,  4, matrix.Color(255,255,255));
  
  matrix.show();
  
  // LED strip: dim white idle light
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 50, 50, 50);  // Dim white
  }
  strip.show();
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/0000mm08.mp3");
  }
  
  // Smooth arm reset
  int leftTarget  = 60;
  int rightTarget = 100;
  int smoothSteps = 60;   // Extra smooth
  int delayTime   = 15;   // Slower motion
  int leftStart  = LeftArm.read();
  int rightStart = RightArm.read();
  
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(leftStart + (leftTarget - leftStart) * t);
    RightArm.write(rightStart + (rightTarget - rightStart) * t);
    delay(delayTime);
  }
  
  break;
}


case 9:
{
  Serial.println(F("Case 9"));

  matrix.clear();

  // Left eye
  for (int x = 2; x < 6; x++)
    for (int y = 2; y < 6; y++)
      matrix.drawPixel(x, y, matrix.Color(255,255,255));

  // Right eye
  for (int x = 10; x < 14; x++)
    for (int y = 2; y < 6; y++)
      matrix.drawPixel(x, y, matrix.Color(255,255,255));

  matrix.show();

  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000009.mp3");
  }
  
  // Antenna spin
  for (int round = 0; round < 1; round++) {
    for (int i = 0; i <= 180; i++) {
      Antenna.write(i);
      delay(8);
    }
    for (int i = 180; i >= 0; i--) {
      Antenna.write(i);
      delay(8);
    }
  }

  // Return to center (90°)
  Antenna.write(90);
  delay(100);

  break;
}


// CASE 10: sad eyes (╯╰)
case 10:
{
  Serial.println(F("Case 10"));

  matrix.clear();

  // Left eye ╯
  matrix.drawPixel(1, 6, matrix.Color(255,255,255));
  matrix.drawPixel(2, 6, matrix.Color(255,255,255));
  matrix.drawPixel(3, 5, matrix.Color(255,255,255));
  matrix.drawPixel(4, 5, matrix.Color(255,255,255));
  matrix.drawPixel(5, 4, matrix.Color(255,255,255));
  matrix.drawPixel(6, 4, matrix.Color(255,255,255));

  // Right eye ╰
  matrix.drawPixel(14, 6, matrix.Color(255,255,255));
  matrix.drawPixel(13, 6, matrix.Color(255,255,255));
  matrix.drawPixel(12, 5, matrix.Color(255,255,255));
  matrix.drawPixel(11, 5, matrix.Color(255,255,255));
  matrix.drawPixel(10, 4, matrix.Color(255,255,255));
  matrix.drawPixel(9,  4, matrix.Color(255,255,255));

  matrix.show();
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000n10.mp3");
  }
  
  // sound (alt, disabled)
}
// CASE 11: eye reset (square eyes), antenna spin
case 11:
{
  Serial.println(F("Case 11"));

  matrix.clear();

  // Left square eye
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255,255,255));
    }
  }

  // Right square eye
  for (int x = 10; x < 14; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255,255,255));
    }
  }

  matrix.show();
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000011.mp3");
  }

  // Antenna full rotation
  for (int round = 0; round < 1; round++) {
    for (int i = 0; i <= 180; i++) {
      Antenna.write(i);
      delay(8);
    }
    for (int i = 180; i >= 0; i--) {
      Antenna.write(i);
      delay(8);
    }
  }
  
  // Return to 90°
  Antenna.write(90);
  delay(100);
  break;
}

// CASE 12: ^^ eyes, synchronized arm swings
case 12:
{
  Serial.println(F("Case 12"));

  matrix.clear();

  // ^^ eyes
  // left ^^ 
  matrix.drawPixel(1, 4, matrix.Color(255,255,255));
  matrix.drawPixel(2, 3, matrix.Color(255,255,255));
  matrix.drawPixel(3, 2, matrix.Color(255,255,255));
  matrix.drawPixel(4, 1, matrix.Color(255,255,255));
  matrix.drawPixel(5, 2, matrix.Color(255,255,255));
  matrix.drawPixel(6, 3, matrix.Color(255,255,255));
  matrix.drawPixel(7, 4, matrix.Color(255,255,255));
  
  // right ^^
  matrix.drawPixel(9, 4, matrix.Color(255,255,255));
  matrix.drawPixel(10, 3, matrix.Color(255,255,255));
  matrix.drawPixel(11, 2, matrix.Color(255,255,255));
  matrix.drawPixel(12, 1, matrix.Color(255,255,255));
  matrix.drawPixel(13, 2, matrix.Color(255,255,255));
  matrix.drawPixel(14, 3, matrix.Color(255,255,255));
  matrix.drawPixel(15, 4, matrix.Color(255,255,255));

  matrix.show();

  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000012.mp3");
  }

// Arms
int leftCenter = 60;
int rightCenter = 100;
int armStep = 30;
int smoothSteps = 40;

// Forward
for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(leftCenter - armStep * t);
    RightArm.write(rightCenter + armStep * t);
    delay(20);
}
delay(100);

// Backward
for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write((leftCenter - armStep) + armStep * t * 2);
    RightArm.write((rightCenter + armStep) - armStep * t * 2);
    delay(20);
}
delay(100);

// Forward
for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write((leftCenter + armStep) - armStep * t * 2);
    RightArm.write((rightCenter - armStep) + armStep * t * 2);
    delay(20);
}
delay(100);

// Back to center
for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write((leftCenter - armStep) + armStep * t);
    RightArm.write((rightCenter + armStep) - armStep * t);
    delay(20);
}

  break;
}

// CASE 13: eye reset
case 13:
{
  Serial.println(F("Case 13"));

  matrix.clear();

  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255,255,255));
      matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));
    }
  }

  matrix.show();
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000013.mp3");
  }
  break;
}

// CASE 14: eye reset, antenna rotation (single loop)
case 14:
{
  Serial.println(F("Case 14"));

  matrix.clear();

  // Left and right square eyes
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255,255,255));
      matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));
    }
  }

  matrix.show();
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000014.mp3");
  }

  // Antenna full rotation
  for (int round = 0; round < 1; round++) {
    for (int i = 0; i <= 180; i++) {
      Antenna.write(i);
      delay(8);
    }
    for (int i = 180; i >= 0; i--) {
      Antenna.write(i);
      delay(8);
    }
  }

  // Return to 90°
  Antenna.write(90);

  break;
}

// CASE 15: eye reset, raise left arm 45°
case 15:
{
  Serial.println(F("Case 15"));

  matrix.clear();

  // Square eyes
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255,255,255));
      matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));
    }
  }
  matrix.show();

  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000015.mp3");
  }

  int leftCenter  = 60;
  int target      = leftCenter - 45;  // Upward motion = decreasing angle
  int smoothSteps = 15;

  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(leftCenter + (target - leftCenter) * t);
    delay(20);
  }

  break;
}

// CASE 16: "- -" eyes, raise left arm 90°
case 16:
{
  Serial.println(F("Case 16"));

  matrix.clear();

  // "- -" eyes
  for (int x = 1; x <= 6; x++) {
    matrix.drawPixel(x, 3, matrix.Color(255,255,255));
    matrix.drawPixel(x, 4, matrix.Color(255,255,255));
    matrix.drawPixel(x + 8, 3, matrix.Color(255,255,255));
    matrix.drawPixel(x + 8, 4, matrix.Color(255,255,255));
  }
  matrix.show();

  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000016.mp3");
  }

  int leftCenter  = 60;
  int target      = leftCenter - 90; 
  int smoothSteps = 40;

  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(leftCenter + (target - leftCenter) * t);
    delay(20);
  }

  break;
}


// CASE 17: eye reset
case 17: {
  Serial.println(F("Case 17"));
  
  matrix.clear();
  
  // Square eyes shifted to bottom-right, separated
  // Left eye: bottom-right, slightly left
  for (int x = 4; x < 8; x++) {
    for (int y = 4; y < 8; y++) {
      matrix.drawPixel(x, y, matrix.Color(255,255,255));
    }
  }
  
  // Right eye: bottom-right, slightly right
  for (int x = 12; x < 16; x++) {
    for (int y = 4; y < 8; y++) {
      matrix.drawPixel(x, y, matrix.Color(255,255,255));
    }
  }
  
  matrix.show();
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000017.mp3");
  }
  
  break;
}


case 18:
{
  Serial.println(F("Case 18"));

  matrix.clear();

  // ===============================
  // Angry eyes (center low, sides high)
  // ===============================

  // Left eye (╮)
  matrix.drawPixel(1, 4, matrix.Color(255,255,255));
  matrix.drawPixel(2, 4, matrix.Color(255,255,255));
  matrix.drawPixel(3, 5, matrix.Color(255,255,255));
  matrix.drawPixel(4, 5, matrix.Color(255,255,255));
  matrix.drawPixel(5, 6, matrix.Color(255,255,255));
  matrix.drawPixel(6, 6, matrix.Color(255,255,255));

  // Right eye (╭)
  matrix.drawPixel(9, 6, matrix.Color(255,255,255));
  matrix.drawPixel(10, 6, matrix.Color(255,255,255));
  matrix.drawPixel(11, 5, matrix.Color(255,255,255));
  matrix.drawPixel(12, 5, matrix.Color(255,255,255));
  matrix.drawPixel(13, 4, matrix.Color(255,255,255));
  matrix.drawPixel(14, 4, matrix.Color(255,255,255));

  matrix.show();

  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000018.mp3");
  }

  // ===============================
  // Angry left-arm swing (90° front → 30° back)
  // ===============================
  int leftCenter = 60;               // Neutral hanging position
  int startPos   = leftCenter - 90;  // Current raised position
  int target     = leftCenter + 30;  // Backward swing
  int smoothSteps = 30;

  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(startPos + (target - startPos) * t);
    delay(20);
  }

  break;
}


// CASE 19: same as case 18
case 19: {
  Serial.println(F("Case 19"));
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000019.mp3");
  }
  
  // Continuous angry red blinking
  Serial.println(F("Red light blinking angrily..."));
  unsigned long lastBlink = 0;
  bool isOn = false;
  
  // Blink continuously until a new case is received
  while (true) {
    unsigned long now = millis();
    
    // Check for new radio signal
    radio.startListening();
    if (radio.available(&pipeNum)) {
      radio.read(&data, sizeof(data));
      // Exit loop if a new case is received
      if (data.stateNumber != 19) {
        break;
      }
    }
    
    // Fast blinking (toggle every 250 ms)
    if (now - lastBlink > 250) {
      isOn = !isOn;
      if (isOn) {
        // ON (red)
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, 30, 0, 0);  // Medium red
        }
      } else {
        // OFF
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
      }
      strip.show();
      lastBlink = now;
    }
    
    delay(10);  // Small delay to reduce CPU usage
  }
  
  break;
}

// CASE 20: eye reset, left arm reset
case 20: {
  Serial.println(F("Case 20"));
  
  matrix.clear();
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255,255,255));
      matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));
    }
  }
  matrix.show();
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000020.mp3");
  }
  
  // Initialize flicker timer
  lastDiscoUpdate = millis();
  
  int leftCenter = 60;
  int smoothSteps = 40;
  
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(LeftArm.read() + (leftCenter - LeftArm.read()) * t);
    
    // Random flicker effect (lower update rate to avoid freezing)
    unsigned long now = millis();
    if (now - lastDiscoUpdate > discoInterval) {
      lastDiscoUpdate = now;
      for (int j = 0; j < LED_COUNT; j++) {
        strip.setPixelColor(j,
          random(20, 60),
          random(20, 60),
          random(20, 60)
        );
      }
      strip.show();
    }
    
    delay(20);
  }
  
  break;
}

//CASE 21 eyes "- -", head shakes left and right
case 21: {
  Serial.println(F("Case 21"));
  
  matrix.clear();
  
  // eyes "- -"
  // left:
  matrix.drawPixel(1, 3, matrix.Color(255,255,255));
  matrix.drawPixel(2, 3, matrix.Color(255,255,255));
  matrix.drawPixel(3, 3, matrix.Color(255,255,255));
  matrix.drawPixel(4, 3, matrix.Color(255,255,255));
  matrix.drawPixel(5, 3, matrix.Color(255,255,255));
  matrix.drawPixel(6, 3, matrix.Color(255,255,255));
  
  matrix.drawPixel(1, 4, matrix.Color(255,255,255));
  matrix.drawPixel(2, 4, matrix.Color(255,255,255));
  matrix.drawPixel(3, 4, matrix.Color(255,255,255));
  matrix.drawPixel(4, 4, matrix.Color(255,255,255));
  matrix.drawPixel(5, 4, matrix.Color(255,255,255));
  matrix.drawPixel(6, 4, matrix.Color(255,255,255));
  
  // right 
  // row 3 (y = 3)
  matrix.drawPixel(9, 3, matrix.Color(255,255,255));
  matrix.drawPixel(10, 3, matrix.Color(255,255,255));
  matrix.drawPixel(11, 3, matrix.Color(255,255,255));
  matrix.drawPixel(12, 3, matrix.Color(255,255,255));
  matrix.drawPixel(13, 3, matrix.Color(255,255,255));
  matrix.drawPixel(14, 3, matrix.Color(255,255,255));
  
  // row 4 (y = 4)
  matrix.drawPixel(9, 4, matrix.Color(255,255,255));
  matrix.drawPixel(10, 4, matrix.Color(255,255,255));
  matrix.drawPixel(11, 4, matrix.Color(255,255,255));
  matrix.drawPixel(12, 4, matrix.Color(255,255,255));
  matrix.drawPixel(13, 4, matrix.Color(255,255,255));
  matrix.drawPixel(14, 4, matrix.Color(255,255,255));
  
  matrix.show();
  
  // LED strip - restore dim white light
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 50, 50, 50);  // dim white
  }
  strip.show();
  Serial.println(F("White light restored"));
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000021.mp3");
  }
  
  // head movement
  for (int pos = 90; pos >= 60; pos--) {
    Head.write(pos);
    delay(20);
  }
  
  for (int pos = 70; pos <= 120; pos++) {
    Head.write(pos);
    delay(20);
  }
  
  for (int pos = 120; pos >= 90; pos--) {
    Head.write(pos);
    delay(20);
  }
  
  break;
}
//CASE 22 eyes move left and right
case 22:
{
  Serial.println(F("Case 22"));
   // sound
 if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000022.mp3");
  }
  // ====== eyeball left-right animation ======
matrix.clear();

// ---------- initial position (center) ----------
for (int x = 2; x < 6; x++) {
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));       // left eye
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));   // right eye
  }
}
matrix.show();
delay(300);


// ---------- eyeballs move left ----------
matrix.clear();
for (int x = 1; x < 5; x++) {        // ← shift left by 1
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));       
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));   
  }
}
matrix.show();
delay(300);

// ---------- eyeballs move left ----------
matrix.clear();
for (int x = 1; x < 4; x++) {        // ← shift left by 2
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));       
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));   
  }
}
matrix.show();
delay(300);

// ---------- eyeballs move left ----------
matrix.clear();
for (int x = 1; x < 5; x++) {        // ← shift left by 1
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));       
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));   
  }
}
matrix.show();
delay(300);

// ---------- initial position (center) ----------
for (int x = 2; x < 6; x++) {
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));       // left eye
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));   // right eye
  }
}
matrix.show();
delay(300);

// ---------- eyeballs move right ----------
matrix.clear();
for (int x = 3; x < 7; x++) {        // → shift right by 1
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));
  }
}
matrix.show();
delay(300);

// ---------- eyeballs move right ----------
matrix.clear();
for (int x = 3; x < 8; x++) {        // → shift right by 2
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));
  }
}
matrix.show();
delay(300);

// ---------- eyeballs move right ----------
matrix.clear();
for (int x = 3; x < 7; x++) {        // → shift right by 1
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));
  }
}
matrix.show();
delay(300);

// ---------- back to center ----------
matrix.clear();
for (int x = 2; x < 6; x++) {
  for (int y = 2; y < 6; y++) {
    matrix.drawPixel(x, y, matrix.Color(255,255,255));
    matrix.drawPixel(x + 8, y, matrix.Color(255,255,255));
  }
}
matrix.show();
delay(300);

  break;
}
//CASE 23 eyes ^^, left arm raises to point at judge, then lowers
case 23:
{
  Serial.println(F("Case 23"));
  
  matrix.clear();
  // left eye ^^
  matrix.drawPixel(1, 4, matrix.Color(255,255,255));
  matrix.drawPixel(2, 3, matrix.Color(255,255,255));
  matrix.drawPixel(3, 2, matrix.Color(255,255,255));
  matrix.drawPixel(4, 1, matrix.Color(255,255,255));
  matrix.drawPixel(5, 2, matrix.Color(255,255,255));
  matrix.drawPixel(6, 3, matrix.Color(255,255,255));
  matrix.drawPixel(7, 4, matrix.Color(255,255,255));
  // right eye ^^
  matrix.drawPixel(9, 4, matrix.Color(255,255,255));
  matrix.drawPixel(10, 3, matrix.Color(255,255,255));
  matrix.drawPixel(11, 2, matrix.Color(255,255,255));
  matrix.drawPixel(12, 1, matrix.Color(255,255,255));
  matrix.drawPixel(13, 2, matrix.Color(255,255,255));
  matrix.drawPixel(14, 3, matrix.Color(255,255,255));
  matrix.drawPixel(15, 4, matrix.Color(255,255,255));

  matrix.show();

  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000023.mp3");
  }

  int leftCenter  = 60;         // natural resting
  int raiseAmount = 60;         // raise angle
  int targetUp    = leftCenter - raiseAmount; // 60 - 60 = 0
  int smoothSteps = 40;

  // raise arm
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(leftCenter + (targetUp - leftCenter) * t); // 60 → 0
    delay(20);
  }

  delay(500); // pause while pointing

  // lower arm back to rest
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    LeftArm.write(targetUp + (leftCenter - targetUp) * t); // 0 → 60
    delay(20);
  }

  break;
}

//CASE 24 eyes "- -"
case 24:
{
  Serial.println(F("Case 24"));
  // sound
 if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000024.mp3");
  }
   matrix.clear();
  // eyes "- -"
// left:
matrix.drawPixel(1, 3, matrix.Color(255,255,255));
matrix.drawPixel(2, 3, matrix.Color(255,255,255));
matrix.drawPixel(3, 3, matrix.Color(255,255,255));
matrix.drawPixel(4, 3, matrix.Color(255,255,255));
matrix.drawPixel(5, 3, matrix.Color(255,255,255));
matrix.drawPixel(6, 3, matrix.Color(255,255,255));

matrix.drawPixel(1, 4, matrix.Color(255,255,255));
matrix.drawPixel(2, 4, matrix.Color(255,255,255));
matrix.drawPixel(3, 4, matrix.Color(255,255,255));
matrix.drawPixel(4, 4, matrix.Color(255,255,255));
matrix.drawPixel(5, 4, matrix.Color(255,255,255));
matrix.drawPixel(6, 4, matrix.Color(255,255,255));

  // right 
  
// row 3 (y = 3)
matrix.drawPixel(9, 3, matrix.Color(255,255,255));
matrix.drawPixel(10, 3, matrix.Color(255,255,255));
matrix.drawPixel(11, 3, matrix.Color(255,255,255));
matrix.drawPixel(12, 3, matrix.Color(255,255,255));
matrix.drawPixel(13, 3, matrix.Color(255,255,255));
matrix.drawPixel(14, 3, matrix.Color(255,255,255));

// row 4 (y = 4)
matrix.drawPixel(9, 4, matrix.Color(255,255,255));
matrix.drawPixel(10, 4, matrix.Color(255,255,255));
matrix.drawPixel(11, 4, matrix.Color(255,255,255));
matrix.drawPixel(12, 4, matrix.Color(255,255,255));
matrix.drawPixel(13, 4, matrix.Color(255,255,255));
matrix.drawPixel(14, 4, matrix.Color(255,255,255));

matrix.show();

  break;
}
//CASE 25 eyes initialization

case 25:
{
  Serial.println(F("Case 25"));
  // sound
 if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000025.mp3");
  }
  matrix.clear();
  // initial square pattern (centered 4x4)
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255, 255, 255)); 
    }
  }
for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x + 8, y, matrix.Color(255, 255, 255));
    }
  }
  matrix.show();
  
  break;
}
case 26:
{
  Serial.println(F("Case 26"));
  matrix.clear();

  //===============================
  //   angry eyes (low center, high sides)
  //===============================

  // left eye (╮)
  matrix.drawPixel(1, 4, matrix.Color(255,255,255));
  matrix.drawPixel(2, 4, matrix.Color(255,255,255));
  matrix.drawPixel(3, 5, matrix.Color(255,255,255));
  matrix.drawPixel(4, 5, matrix.Color(255,255,255));
  matrix.drawPixel(5, 6, matrix.Color(255,255,255));
  matrix.drawPixel(6, 6, matrix.Color(255,255,255));

  // right eye (╭)
  matrix.drawPixel(9, 6, matrix.Color(255,255,255));
  matrix.drawPixel(10, 6, matrix.Color(255,255,255));
  matrix.drawPixel(11, 5, matrix.Color(255,255,255));
  matrix.drawPixel(12, 5, matrix.Color(255,255,255));
  matrix.drawPixel(13, 4, matrix.Color(255,255,255));
  matrix.drawPixel(14, 4, matrix.Color(255,255,255));

  matrix.show();
  // sound
 if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000026.mp3");
  }
// raise movement
int leftCenter  = 60;
int target      = leftCenter - 45;
int smoothSteps = 15;

for (int i = 0; i <= smoothSteps; i++) {
  float t = i / (float)smoothSteps;
  LeftArm.write(leftCenter + (target - leftCenter) * t);
  delay(20);
}

// ===== non-blocking pause =====
unsigned long pauseStartTime = millis();
unsigned long pauseDuration = 2000;  // pause 2 seconds (2000 ms)

while (millis() - pauseStartTime < pauseDuration) {
  // handle other tasks here if needed
}

// ===== smooth reset =====
for (int i = 0; i <= smoothSteps; i++) {
  float t = i / (float)smoothSteps;
  LeftArm.write(target + (leftCenter - target) * t);
  delay(20);
}

  break;
  
}
// case 27:

case 27: {
  Serial.println(F("Case 27"));
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000027.mp3");
  }
  
  // LED blinking effect (3 times)
  Serial.println(F("LED blinking blue..."));
  int blinkCount = 3;  // blink count
  int blinkDelay = 200;  // interval per blink (ms)
  
  for (int blink = 0; blink < blinkCount; blink++) {
    // on (blue)
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0, 0, 30);  // dim blue
    }
    strip.show();
    delay(blinkDelay);
    
    // off
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
    delay(blinkDelay);
  }
  
  // after blinking, keep blue on
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 0, 0, 30);  // dim blue steady
  }
  strip.show();
  Serial.println(F("Blue light set - starting arm movement..."));
  
  int center = 100;          // start position
  int target = center + 30;  // 100 → 130
  int smoothSteps = 40;
  
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    int armPos = center + (target - center) * t;
    RightArm.write(armPos);
    delay(20);
  }
  
  break;
}
case 28:
{
  Serial.println(F("Case 28"));
  // sound
 if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000028.mp3");
  }
   matrix.clear();
  // initial square pattern (centered 4x4)
  for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x, y, matrix.Color(255, 255, 255)); 
    }
  }
for (int x = 2; x < 6; x++) {
    for (int y = 2; y < 6; y++) {
      matrix.drawPixel(x + 8, y, matrix.Color(255, 255, 255));
    }
  }
  matrix.show();
  // head movement
for (int pos = 90; pos >= 60; pos--) {
  Head.write(pos);
  delay(20);
}
for (int pos = 70; pos <= 120; pos++) {
  Head.write(pos);
  delay(20);
}
for (int pos = 120; pos >= 90; pos--) {
  Head.write(pos);
  delay(20);
}
  break;
}
case 29: {
  Serial.println(F("Case 29"));
  
  // raise arm first
  int startPos = 100;   // current position
  int endPos   = 180;   // highest position
  int smoothSteps = 50; // smoother for high lift
  
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    int armPos = startPos + (endPos - startPos) * t;  // 100 → 180
    RightArm.write(armPos);
    delay(20);
  }
  
  // sound
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000029.mp3");
  }
  
  // crazy blue flashing (dark blue, light blue, white)
  Serial.println(F("Crazy blue light blinking..."));
  unsigned long blinkStartTime = millis();
  unsigned long blinkDuration = 4000;  // flashing duration
  unsigned long lastBlink = 0;
  int blinkState = 0;  // 0=dark blue, 1=light blue, 2=white
  
  while (millis() - blinkStartTime < blinkDuration) {
    unsigned long now = millis();
    
    // switch every 150ms
    if (now - lastBlink > 150) {
      blinkState = (blinkState + 1) % 3;
      
      switch (blinkState) {
        case 0:  // dark blue
          for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, 0, 0, 80);
          }
          break;
        case 1:  // light blue
          for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, 0, 0, 30);
          }
          break;
        case 2:  // white
          for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, 40, 40, 40);
          }
          break;
      }
      strip.show();
      lastBlink = now;
    }
    
    delay(10);
  }
  
  // end flashing, keep dark blue (for case 30)
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 0, 0, 80);
  }
  strip.show();
  Serial.println(F("Blinking stopped, deep blue maintained"));
  
  break;
}
case 30: {
  Serial.println(F("Case 30"));
  
  // steady dark blue
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 0, 0, 80);
  }
  strip.show();
  Serial.println(F("Deep blue light maintained"));
  
  int startPos = 180;   // height from Case 29
  int endPos   = 90;    // slam down position
  int smoothSteps = 35; // slightly faster but smooth
  
  for (int i = 0; i <= smoothSteps; i++) {
    float t = i / (float)smoothSteps;
    int armPos = startPos + (endPos - startPos) * t;  // 180 → 90
    RightArm.write(armPos);
    delay(18);
  }
  
  // sound
  musicPlayer.setVolume(5,5);
  if (!musicPlayer.playingMusic) {
    musicPlayer.startPlayingFile("/00000030.mp3");
  }
  
  break;
}

}
}  // end of loop()
}
// end of receiver code
// CHANGEHERE