#ifndef SENSOR_READ_H
#define SENSOR_READ_H
#include <Arduino.h>

int isTilted(int digital_input);
float lightDetection(int analog_input, float source_voltage, float acdc_max);
bool got_mail(int *ptr_tilt, float *ptr_photo, float analog_treshold);
void main_fun(bool *flap_state, bool *isOpen, bool *isClosed);


#endif
