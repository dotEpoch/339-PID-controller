/*
DC Fan Control Arduino Program
--> Author: Vincent Aucoin @dotEpoch
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
// Packages
#include "Adafruit_MAX31856.h" // Make sure to install this library!
/*#include <AccelStepper.h> // Unused since fan is DC*/
#include <Adafruit_MotorShield.h>

// Definitions
#define SKETCH_VERSION "0.0.1"
#define BAUD 115200

// Init Thermocouple
Adafruit_MAX31856 thermocouple = Adafruit_MAX31856(5,4,3,2); // Use software SPI: CS, DI, DO, CLK

// Init Fan Motor 
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); // Grab motorShield
Adafruit_DCMotor *myMotor = AFMS.getMotor(1); // get motor on port 1

// Constants
uint32_t t0 = 0;         // Reference time
uint16_t heater = 0;  // Heater power to 0 for maximum cooling
const float cutoffTemp = 50.0; // Temperature cut-off point.
///////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  // --- ThermoCouple --- //
  Serial.begin(BAUD);      // Enable Serial COM
  thermocouple.begin();    // Enable MAX31856 

  thermocouple.setThermocoupleType(MAX31856_TCTYPE_K);  // Set thermocouple as K type 
  thermocouple.setConversionMode(MAX31856_CONTINUOUS);  // Use continuous conversion mode
  
  /* Configure pin 9 for slow (1Hz) PWM (DO NOT MODIFY THIS!)*/
  pinMode(9, OUTPUT);                 
  TCCR1A = _BV(COM1A1) | _BV(WGM11);  // Enable the PWM output OC1A (Arduino digital pin 9)
  TCCR1B = _BV(WGM13) | _BV(WGM12);   // Set fast pwm mode w/ ICR1 as top
  TCCR1B = TCCR1B | _BV(CS12);        // Set prescaler @ 256
  ICR1 = 62499;                       // Set the PWM frequency to 1Hz: 16MHz/(256 * 1Hz) - 1 = 62499
  OCR1A = 0;                          // Output compare register for setting duty cycle (This can be conveniently set with analogWrite())
  
  t0 = millis();                      // Set start time 
  analogWrite(9,heater);              // Set a fixed heater power

  // --- Fan Control --- //
  AFMS.begin(50);

  myMotor->run(FORWARD);
  for (uint8_t i=0; i<255; i++) {
    myMotor->setSpeed(i);
    delay(10);
  }
}

void loop() {

  float temperature = thermocouple.readThermocoupleTemperature(); // Read the last temperature measurement made by the thermocouple
  delay(100); // 100 ms delay to give the MAX31856 time for next temperature conversion (see datasheet!)

  /* Print out time, temperature, and heater data (comma delimited)*/
  Serial.print(millis()-t0);
  Serial.print(",");
  Serial.print(String(temperature,4));
  Serial.print(",");
  Serial.println(heater); 


  if (temperature > cutoffTemp) {
    analogWrite(9, 0);
  } 
  else {
    analogWrite(9, heater);
  }

  if (temperature < 24) {
    myMotor->run(RELEASE);
  }


  //delay(1000);
}
