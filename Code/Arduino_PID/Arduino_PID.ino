/*
 * Example script for use in the PHYS-339 PID lab at McGill University.
 * Written by Brandon Ruffolo in Feb 2024.
*/

/* Packages */
#include "Adafruit_MAX31856.h" // Make sure to install this library!
#include <Adafruit_MotorShield.h>

/* Definitions */
#define SKETCH_VERSION "0.0.1"
#define BAUD 115200

/* Init Thermocouple */
Adafruit_MAX31856 thermocouple = Adafruit_MAX31856(5,4,3,2); // Use software SPI: CS, DI, DO, CLK

/* Init Fan Motor */
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); // Grab motorShield
Adafruit_DCMotor *myMotor = AFMS.getMotor(1); // get motor on port 1

/* Init Constants */
uint32_t t0 = 0;         // Reference time
uint16_t heater = 50000;  // Heater power
const float cutoffTemp = 150.0; // Temperature cut-off point.
bool cooling = true;
//Proportional Band
bool tempReached = false;
const float targetTemp = 90.0;
const float bandRange = 5.0;
const float upperBound = targetTemp + bandRange/2.0;
const float lowerBound = targetTemp - bandRange/2.0;
const int interval = 2; // after "interval" points, reset integral
uint32_t past = 0;


void setup() {
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
    
  t0 = millis();                  // Set start time 
  analogWrite(9,heater);              // Set a fixed heater power
  past = t0; 

    // --- Fan Control --- //
  AFMS.begin(50);

  myMotor->run(RELEASE);
  for (uint8_t i=0; i<255; i++) {
    myMotor->setSpeed(i);
    delay(10);
  }
}

float integrate(float *temperatureList, int count, float a, float b) { //trapezoidal rule
  float h = (b-a)/((count));
  // Serial.print(h);
  // Serial.print("<--h,  ");
  float integral = 0;
  for (int pos=0; pos <= count; pos++){
    if (pos == 0 || pos == count){
      integral = integral + h/2.0*temperatureList[pos];
    }
    else{
        integral = integral + h*temperatureList[pos];
        temperatureList[pos] = 0;
    }
  }
  return integral/1000;
}

float derivative() {



  return 0.0;
}


void loop() {

  float temperature = thermocouple.readThermocoupleTemperature(); // Read the last temperature measurement made by the thermocouple
  delay(100); // 100 ms delay to give the MAX31856 time for next temperature conversion (see datasheet!)
  cooling = false; //for debugging
  if (cooling && temperature < 25) { // If cooling is needed
    myMotor->run(RELEASE);
    cooling = false;
    //analogWrite(9, heater);
    heater = 50000;
  }
  else if (!cooling ) { //&& tempReached

    if (temperature > cutoffTemp) {
      heater = 0;
      delay(5000);
    } 
    else if ( lowerBound < temperature && temperature < upperBound) { // in between bounds

      static float integral = 0;
      static float derivative = 0;
      static float errorBuffer[interval];
      static int count = 0;

      float error = (targetTemp - temperature); // Error of target vs measured
      errorBuffer[count] = error;
      count++;

      if (count == interval-1){
        //update integral term
        float curr = millis();
        integral = integrate(errorBuffer, count, past, curr);
        count = 0;
        past = millis();
      }
      float coeff = (0.5 + error/bandRange  + integral + derivative);  // proportional error + integral + derivative
      float proportionalHeat = abs(coeff) * 50000;
      
      // Serial.print(coeff);
      // Serial.print("<--Coeff,  ");
      // Serial.print(error/bandRange);
      // Serial.print("<--Error,  ");
      // Serial.print(integral);
      // Serial.print("<--Integral ||  ");

      heater = proportionalHeat;
    }
    else if (temperature < lowerBound) {
      heater = 50000;
    }
    else if (temperature > upperBound) {
      heater = 0;
    }

  }
  analogWrite(9, heater); 

  /* Print out time, temperature, and heater data (comma delimited)*/
  Serial.print(millis()-t0);
  Serial.print(",");
  Serial.print(String(temperature,4));
  Serial.print(",");
  Serial.println(heater); 

  //if (temperature >= targetTemp) tempReached =true;

  //delay(1000);
}
