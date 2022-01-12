/*
 Project work for course 34338 - Telecommunication programming projects with Arduino January 2022
 Sensor part by Krzysztof Jan Pac
 
 */
 
//pin definition on ESP8266
#define INPUT_D2 D2
#define INPUT_A0 A0

const int analog_input = INPUT_A0;
const int digital_input = INPUT_D2;
const float analog_treshold = 0.20; //treshold in V to see if postal box flap is open
const float source_voltage = 3.3; //source input voltage
const float acdc_max = 1023.0; //maximum value obtained form AC/DC converter

//function detecting if flap has changed it's position
int isTilted();
//fucntions which detects the change of the intensity of light
float lightDetection();
//checks if conditions are met 
bool got_mail(int* ptr1, int *ptr2);
//main function, sends notification
void main_fun(bool *flap_state, bool *isOpen, bool *isClosed);

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
 tilt_sensor = isTilted();
 photo_sensor = lightDetection();
 box_state = got_mail(ptr_tilt, ptr_photo);
 main_fun(&box_state, ptr_sw1, ptr_sw2);
 

}

//functions declaration:

int isTilted()
{
  int sensor_input = digitalRead(digital_input);
  return sensor_input;
}

float lightDetection()
{
  float sensor_input = analogRead(analog_input);
  float input_volatge = sensor_input * (source_voltage / acdc_max);
  return input_volatge;
}

bool got_mail(int* ptr1, float *ptr2)
{
  static bool postal_box_state = 0;
  if((*ptr1 == 1) && (*ptr2 > analog_treshold))
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
