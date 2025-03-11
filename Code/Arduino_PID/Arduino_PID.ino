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
uint16_t heater = 0;  // Heater power
const float cutoffTemp = 150.0; // Temperature cut-off point.
bool cooling = true;
//Proportional Band
bool tempReached = false;
const float targetTemp = 100.0;
const float bandRange = 8.0;
const float upperBound = targetTemp + bandRange/2.0;
const float lowerBound = targetTemp - bandRange/2.0;
const int interval = 2; // after "interval" points, reset integral
const float T_i = 30000.0;
const float T_d = 1.0;



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

  myMotor->run(FORWARD);
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
  return integral;
}

float derivative(float past[], float prev[], float curr[]) { //three point central difference because first difference differantiator introduces too much noise
  float pastTemp = past[0];
  float pastTime = past[1];

  float prevTemp = prev[0];
  float prevTime = prev[1];

  float currTemp = curr[0];
  float currTime = curr[1];

  float derivative = (prevTemp-pastTemp)(currTime - prevTime)/( (prevTime - pastTime)(currTime - pastTime) ) + (prevTemp - currTemp)(prevTime - pastTime)/( (prevTime - currTime)(currTime - pastTime) );

  return derivative;
}


void loop() {
  
  
  float temperature = thermocouple.readThermocoupleTemperature(); // Read the last temperature measurement made by the thermocouple
  static float past[] = {0.0, 0.0};
  static float prev[] = {0.0, 0.0};
  static float curr[] = {temperature, millis()-t0};


  float curr = millis()-t0;
  static float past = 0.0;
  float error = (targetTemp - temperature)/bandRange;
  static float prevTemp = 0.0;
  static float prevErr = 0.0;
  delay(100); // 100 ms delay to give the MAX31856 time for next temperature conversion (see datasheet!)

  //cooling = false; //for debugging
  if (cooling && temperature < 70) { // If cooling is needed
    myMotor->run(RELEASE);
    cooling = false;
    //analogWrite(9, heater);
    heater = 62498;
  }
  else if (!cooling ) { //&& tempReached

    if (temperature > cutoffTemp) {
      heater = 0;
      delay(5000);
    } 
    else if ( lowerBound < temperature && temperature < upperBound) { // in between bounds

      static float integral = 0.0;
      static float derivative = 0.0;
      //static float derivBuffer[];

      float error = (targetTemp - temperature)/bandRange; // Error of target vs measured
      integral = error*(curr-past) + integral;
      derivative = derivative - (error - prevErr) / (curr - past);

      float coeff = (0.5 + error + integral/T_i + derivative*T_d);  // proportional error + integral + derivative
      float proportionalHeat = abs(coeff) * 62498;
      
      // Serial.print(coeff, 4);
      // Serial.print("<--Coeff,  ");
      // Serial.print(error, 4);
      // Serial.print("<--Error,  ");
      // Serial.print(integral/T_i, 4);
      // Serial.print("<--Integral ||  ");
      // Serial.print(derivative, 4);
      // Serial.print("<--derivative ||  ");


      heater = proportionalHeat;
    }
    else if (temperature < lowerBound) {
      heater = 62498;
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
  prevTemp = temperature;
  prevErr = error;
  past = curr;
}
