//
//    FILE: main.cpp
//  AUTHOR: Joe Andolina
//    DATE: 1-25-2025
// VERSION: 0.1.0
// PURPOSE: Code for the Variable Length Tube Cutter
//    REPO: https://github.com/Phando/VariableTubeCutter

// NOTE: The stepper motor in this project is the Things by Josh PD-Stepper
//       it has an integrated esp32, buttons and STEMMA bus 
// REPO: https://github.com/joshr120/PD-Stepper


#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <AceButton.h>
#include <Adafruit_seesaw.h>
#include <seesaw_neopixel.h>
#include <TMC2209.h>
#include <Preferences.h>

#include <algorithm> 
#include <iostream>
#include <string>

using namespace std;
using namespace ace_button;

// Global Variable Defines
Preferences prefs;
Adafruit_seesaw ss;
seesaw_NeoPixel sspixel = seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800);

// Runtime Variables
long current_steps = 0;
int speedMult = 0;

unsigned long lastAutomation = 0;
unsigned long automationDelay = 1000;

bool startMotor = false;
bool motorDir = false; //Stepper motor direction
int lengthMult = LENGTH_MIN; 

long lastFlash = 0;
int flashInt = 500;
int flashOnInt = 25;
int flashOffInt = 2000;
bool flashState = 0;

// TMC2209 Stepper Driver
const int STEPS_PER_REV = 200;   // NEMA17 standard
const int MICROSTEPS = 16;       // 1/16 microstepping
const float DEG_PER_STEP = 360.0 / (STEPS_PER_REV * MICROSTEPS);

// Rotary values
unsigned long lastRotaryTime = 0;
unsigned long rotaryDelay = 100;
bool buttonState = true;

// ButtonConfig buttonConfig;
AceButton button1(SW1);
AceButton button2(SW2);
AceButton button3(SW3);
void handleButton(AceButton*, uint8_t, uint8_t);

//-----------------------------------------------------------------------------

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return sspixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return sspixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return sspixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//-----------------------------------------------------------------------------
void stopAutomation(){
  lastAutomation = 0;
}

//-----------------------------------------------------------------------------
void showFeedback(){
  for(int i =0; i < 5; i++){
    sspixel.setPixelColor(0, 0xFF0000);
    sspixel.show();
    digitalWrite(LED2, HIGH);
    delay(25);
    digitalWrite(LED2, LOW);
    sspixel.setPixelColor(0, 0x00);
    sspixel.show();
    delay(500);
  }

  // Reset the rotary color
  sspixel.setPixelColor(0, Wheel((lengthMult*5) & 0xFF));
  sspixel.show();
}

//-----------------------------------------------------------------------------

void getPrefs(){
  if (!prefs.begin("myapp", true)) {
    cout << "Failed to initialize Preferences!" << endl;
    return;
  }

  motorDir   = prefs.getBool("motorDir", false); 
  speedMult = prefs.getInt("speedMult", SPEED_MAX); 
  lengthMult = prefs.getInt("lengthMult", LENGTH_MIN);  
  prefs.end();

  cout << "Loaded values:" << endl;
  cout << "speedMult: " << speedMult << endl;
  cout << "MotorDir: " << motorDir << endl;
  cout << "LengthMulti: " << lengthMult << endl;
}

//-----------------------------------------------------------------------------

void setPrefs(){
  if (!prefs.begin("myapp", false)) {
    cout << "Failed to initialize Preferences!" << endl;
    return;
  }

  prefs.putInt("speedMult", speedMult);
  prefs.putBool("motorDir", motorDir);
  prefs.putInt("lengthMult", lengthMult);
  prefs.end();

  cout << "Saved values:" << endl;
  cout << "speedMult: " << speedMult << endl;
  cout << "MotorDir: " << motorDir << endl;
  cout << "LengthMulti: " << lengthMult << endl;
}


//-----------------------------------------------------------------------------
// Note: This is a blocking function but can/will be written as non-blocking
// in the next version
//-----------------------------------------------------------------------------

void rotateDegrees(float degrees) {
  // Calculate effective movement (compensate for full rotations)
  float effective_deg = fmod(degrees, 360.0);
  if (effective_deg < 0) effective_deg += 360.0;

  // Calculate required steps
  long target_steps = round(effective_deg / DEG_PER_STEP);
  cout << "Starting Motor" << endl;
  
  // Set direction
  degrees = motorDir ? degrees : -degrees;
  digitalWrite(DIR, (degrees >= 0) ? HIGH : LOW);
  digitalWrite(TMC_EN, LOW); // Enable motor driver
  delayMicroseconds(10);  // DIR setup time

  // Generate steps
  for(long i = 0; i < target_steps; i++) {
    digitalWrite(STEP, HIGH);
    delayMicroseconds(50);  // Minimum 100ns pulse width
    digitalWrite(STEP, LOW);
    delayMicroseconds(speedMult);  // Controls speed
    
    // Track position (wrap at full steps)
    current_steps += (degrees >= 0) ? 1 : -1;
    current_steps %= (STEPS_PER_REV * MICROSTEPS);
  }

  digitalWrite(TMC_EN, HIGH); // Disable motor driver
}

//-----------------------------------------------------------------------------
void activateCutter() { 
  // TODO : Activate the cutter
}

//-----------------------------------------------------------------------------

void handleButton(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  
  // TODO : Start automatic operation
  if(eventType != AceButton::kEventLongPressed){
    lastAutomation = millis();
    return;
  }

  if(eventType != AceButton::kEventReleased){
    return;
  }

  switch (button->getPin()) {
    case SW1:
      speedMult = min(speedMult+SPEED_INC, SPEED_MAX);
      break;
    case SW2:
      motorDir = !motorDir; 
      showFeedback();
      break;
    case SW3:
      speedMult = max(SPEED_MIN,speedMult-SPEED_INC);
      break;
  }
  
  if(speedMult == SPEED_MIN || speedMult == SPEED_MAX){
    showFeedback();
  }

  stopAutomation();
  setPrefs();
}


//-----------------------------------------------------------------------------

void rotaryLoop() {
  if (buttonState != ss.digitalRead(SS_SWITCH)) {
    buttonState = ss.digitalRead(SS_SWITCH);
    startMotor = buttonState == HIGH ;
  }

  int32_t new_position = -ss.getEncoderPosition();
  
  if (lengthMult != new_position) {
    lastRotaryTime = 0; // Stop automation if running
    lengthMult = max(LENGTH_MIN, min(new_position, LENGTH_MAX));
    if(lengthMult == LENGTH_MIN || lengthMult == LENGTH_MAX){
      showFeedback();
    }

    ss.setEncoderPosition(-lengthMult);
    sspixel.setPixelColor(0, Wheel(lengthMult & 0xFF));
    sspixel.show();
    
    setPrefs();
  }
}

//-----------------------------------------------------------------------------

void setupSeeeSaw(){
  Wire.begin();
  if (! ss.begin(SEESAW_ADDR) || ! sspixel.begin(SEESAW_ADDR)) {
    cout << "ERROR: seesaw not found at 0x36!" << endl;
    showFeedback();
    while (1) delay(100);
  }

  cout << "FOUND: seesaw encoder on " << SEESAW_ADDRESS << endl;
  
  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  if (version  != 4991){
    cout << "Wrong firmware loaded " << version << endl;
    while(1) delay(100);
  }
  cout << "Found Product 4991" << endl;

  // set not so bright!
  sspixel.setBrightness(20);
  sspixel.setPixelColor(1,0x0);
  sspixel.show();
  
  // use a pin for the built in encoder switch
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);

  // get starting position
  ss.setEncoderPosition(LENGTH_MIN);
  ss.setEncoderPosition(-lengthMult);

  delay(50);
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();
 
  cout << "Seesaw initialized" << endl;
}

//-----------------------------------------------------------------------------

void setupStepper(){
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
  //PD Trigger Setup
  pinMode(PG, INPUT);
  pinMode(CFG1, OUTPUT);
  pinMode(CFG2, OUTPUT);
  pinMode(CFG3, OUTPUT);
                            //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
  digitalWrite(CFG1, HIGH); //  1    0     0     0     0
  digitalWrite(CFG2, LOW);  //  -    0     0     1     1
  digitalWrite(CFG3, LOW);  //  -    0     1     1     0
  
  // Stepper simple setup (no serial comms)
  pinMode(TMC_EN, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS1, OUTPUT);

  digitalWrite(TMC_EN, HIGH); // High to disable on startup
  digitalWrite(MS1, LOW);     // Microstep resolution configuration (internal pull-down resistors: MS2, MS1: 00: 1/8, 01: 1/32, 10: 1/64 11: 1/16
  digitalWrite(MS2, HIGH);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  // Initialize the buttons
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleButton);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);

  // TODO - Implement LongPress press to start automatic cutting
  // buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  // buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
  
  cout << "Stepper initialized" << endl;
}

//-----------------------------------------------------------------------------

void setup() {
  delay(500);
  cout << " Setup " << endl;
  getPrefs();
  setupStepper();
  setupSeeeSaw();


  // TODO : Implement any special case boot up button press options
  // if (button1.isPressedRaw()) {
  //   Serial.println(F("setup(): button 1 was pressed while booting"));
  // }
  // if (button2.isPressedRaw()) {
  //   Serial.println(F("setup(): button 2 was pressed while booting"));
  // }
  // if (button3.isPressedRaw()) {
  //   Serial.println(F("setup(): button 3 was pressed while booting"));
  // }

  cout << "Starting App" << endl;
}

//-----------------------------------------------------------------------------

void loop() {
  if ((millis() - lastRotaryTime) > rotaryDelay) {
    button1.check();
    button2.check();
    button3.check();
    rotaryLoop();
    lastRotaryTime = millis();
  }

  // TODO : Implelment automation
  // if (lastAutomation != 0 && (millis() - lastAutomation) > automationDelay) {
  //   startMotor = true;
  //   lastRotaryTime = millis();
  // }

  if(startMotor){
    startMotor = false;
    rotateDegrees(lengthMult*LENGTH_INC);
    activateCutter();
  }

  //flash LED to show code is running
  if (millis() - lastFlash > flashInt){
    digitalWrite(LED1, flashState);
    if (flashState == 0){ 
      flashState = 1;
      flashInt = flashOffInt;
    } else {
      flashState = 0;
      flashInt = flashOnInt;
    }
    lastFlash = millis();
  }
}
