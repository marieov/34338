#include "sensor_read.h"

int isTilted(int digital_input)
{
  int sensor_input = digitalRead(digital_input);
  return sensor_input;
}

float lightDetection(int analog_input, float source_voltage, float acdc_max)
{
  float sensor_input = analogRead(analog_input);
  float input_volatge = sensor_input * (source_voltage / acdc_max);
  return input_volatge;
}

bool got_mail(int *ptr_tilt, float *ptr_photo, float analog_treshold)
{
  static bool postal_box_state = 0;
  if((*ptr_tilt == 1) && (*ptr_photo > analog_treshold))
  {
    postal_box_state = 1;
  }
  else
  {
    postal_box_state = 0;
  }

  return postal_box_state;
}

void main_fun(bool *flap_state, bool *isOpen, bool *isClosed)
{
  if(*flap_state == 0)
  {
    *isClosed = 1;
  }
  else
  {
    *isOpen = 1;
  }

  if(*flap_state == 0 && *isOpen == 1 && *isClosed == 1)
  {
    //change when team figure notification feature
    Serial.println("Sending notification");
    *isOpen = 0;
  }
  delay(100);
}
