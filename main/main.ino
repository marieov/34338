/*
 Project work for course 34338 - Telecommunication programming projects with Arduino January 2022
 Work areas:
 Sensor part: Krzysztof Jan Pac
 Mail notificiation: Marie Øverby
 Server: Søren Qvist, Marius Larsen
 */

#include "sensor_read.h"

//pin definition on ESP8266
#define INPUT_D2 D2
#define INPUT_A0 A0

const int analog_input = INPUT_A0;
const int digital_input = INPUT_D2;
const float analog_treshold = 0.20; //treshold in V to see if postal box flap is open
const float source_voltage = 3.3; //source input voltage
const float acdc_max = 1023.0; //maximum value obtained form AC/DC converter

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(digital_input,INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:
 static int tilt_sensor = 0; //variable to store information from tilt sensor
 static float photo_sensor = 0.0; //variable to store information from light sensor
 static int *ptr_tilt = &tilt_sensor; //pointer to tilt sensor
 static float *ptr_photo = &photo_sensor; //pointer to photo sensor
 static bool box_state = 0; //state of the postal box - open or closed
 static bool isOpen_switch = 0; //auxiliary variable - is postal box flap opened
 static bool isClosed_switch = 1; //same as the obove - is closed
 static bool *ptr_sw1 = &isOpen_switch; // pointer to Opened Switch
 static bool *ptr_sw2 = &isClosed_switch; //pointer to Closed Switch
 
//here whole process start
 tilt_sensor = isTilted(digital_input);
 photo_sensor = lightDetection(analog_input, source_voltage, acdc_max);
 box_state = got_mail(ptr_tilt, ptr_photo, analog_treshold);
 main_fun(&box_state, ptr_sw1, ptr_sw2);

}
