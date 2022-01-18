/*
  Project work for course 34338 - Telecommunication programming projects with Arduino January 2022
  Work areas:
  Sensor part: Krzysztof Jan Pac
  Mail notificiation: Marie Øverby
  Server: Søren Qvist, Marius Larsen

  The code consists of three parts:
  Sensor readings
  An email sending service. This code is taken from Rui Santos, https://github.com/mobizt/ESP-Mail-Client, and adapted by Marie Øverby.
  Server with button that delays the email sending service. This is used to avoid getting an email when you collect the mail.

  Minor changes needs to be done in this file in order for the esp to connect to the right WiFi and the email sending service to work.
  Search though the file for "***" to easily find which varibles to change.

*/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <WiFiManager.h>
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ESP8266Webhook.h>

// Create an instance of the server
ESP8266WebServer server(80);

//#define WIFI_SSID "GalxyAP" // *** This needs to be changed according to which wifi is available
//#define WIFI_PASSWORD "password17" // *** This needs to be changed according to which wifi is available

// Pin definition on ESP8266/ESP32
#define INPUT_D2 D2 //ESP32: replace "D2" with "2" ***
#define INPUT_A0 A0 //ESP32: replace "A0" with "0" ***

#define KEY "bMRD9CPHrVrrIVHNTng1NI"        // Webhooks Key
#define EVENT "lol"      // Webhooks Event Name

Webhook webhook(KEY, EVENT);    // Create an object.

// Variables used to read sensor
const int analog_input = INPUT_A0;
const int digital_input = INPUT_D2;
const float analog_treshold = 0.20; //treshold in V to see if postal box flap is open
const float source_voltage = 3.3; //source input voltage
const float acdc_max = 1023.0; //maximum value obtained form AC/DC converter
const unsigned long collection_dealy = 60000; // 60s
static bool want_collect = 0; // store request from the server when user want to collect the mail

void setup() {
  Serial.begin(115200);
  pinMode(digital_input, INPUT_PULLUP);
  serverSetup();
  //delay(500);
}

void loop() {
  // Variables for sensor reading
  static int tilt_sensor = 0; //variable to store information from tilt sensor
  static float photo_sensor = 0.0; //variable to store information from light sensor
  static int *ptr_tilt = &tilt_sensor; //pointer to tilt sensor
  static float *ptr_photo = &photo_sensor; //pointer to photo sensor
  static bool box_state = 0; //state of the postal box - open or closed
  static bool isOpen_switch = 0; //auxiliary variable - is postal box flap opened
  static bool isClosed_switch = 1; //same as the obove - is closed
  static bool *ptr_sw1 = &isOpen_switch; // pointer to Opened Switch
  static bool *ptr_sw2 = &isClosed_switch; //pointer to Closed Switch
  static bool mail_trigger = 0; // variable used to trigger notification feature
  static bool *ptr_trigger = &mail_trigger; // pointer to the above variable
  static unsigned long mytime = 0; // store the value of time that passed since timer has started

  // Check if a client has connected
  server.handleClient();

  /* ------------------- SENSORS -------------------  */
  tilt_sensor = isTilted(digital_input);
  photo_sensor = lightDetection(analog_input, source_voltage, acdc_max);
  box_state = got_mail(ptr_tilt, ptr_photo, analog_treshold);
  mail_trigger = main_fun(&box_state, ptr_sw1, ptr_sw2, ptr_trigger);


  if (*ptr_trigger == 1 && want_collect == 0)
  {
    Serial.println("Send"); //remove later
    sendEmail();
    *ptr_trigger = 0; // reset mail notification trigger to 0
  }
  delay(100);

  // Checks if user want to collect the mail
  if (want_collect == 1)
  {
    // Switch on timer, which disable notification feature for required time
    my_timer(&mytime, &want_collect);
  }

}

/* ------------------- FUNCTIONS FOR SENSOR READING -------------------  */

int isTilted(int digital_input)
{
  int sensor_input = digitalRead(digital_input);
  return sensor_input;
}

float lightDetection(int analog_input, float source_voltage, float acdc_max)
{
  float sensor_input = analogRead(analog_input);
  /*** converting into voltage ***/
  float input_volatge = sensor_input * (source_voltage / acdc_max);
  return input_volatge;
}

bool got_mail(int *ptr_tilt, float *ptr_photo, float analog_treshold)
{
  static bool postal_box_state = 0;
  /*** checks if mailbox flap is/was opened and light illuminance has increased ***/
  if ((*ptr_tilt == 0) && (*ptr_photo > analog_treshold))
  {
    /*** someone opened mailbox/put mail inside ***/
    postal_box_state = 1;
  }
  else
  {
    /*** nothing happened/mailbox is closed right now ***/
    postal_box_state = 0;
  }

  return postal_box_state;
}

bool main_fun(bool *flap_state, bool *isOpen, bool *isClosed, bool *send_trigger)
{
  /*** checks if mailbox's flap/door is opened or closed ***/
  if (*flap_state == 0)
  {
    *isClosed = 1;
  }
  else
  {
    *isOpen = 1;
    *isClosed = 0;
  }

  /*** checks if mailbox's is closed, flap/door was opened and if right now is closed ***/
  if (*flap_state == 0 && *isOpen == 1 && *isClosed == 1)
  {
    /*** if conditions are true, notification trigger is set to be true ***/
    *send_trigger = 1;
    /*** door/flap is closed ***/
    *isOpen = 0;
  }
  return *send_trigger;
}

void my_timer(unsigned long *time_value, bool *ptr_trigger)
{
  /*** start counting time in milliseconds ***/
  *time_value = millis();

  /*** checks if desired time has passed ***/
  if (*time_value >= collection_dealy)
  {
    Serial.println("Time has passed");
    *time_value = 0; // reset counter to the 0
    *ptr_trigger = 0; // switch off button state to false - enables sending mail notification again
  }
}

/* ------------------- EMAIL -------------------  */
void sendEmail() {
  String ip = WiFi.localIP().toString();
 int response = webhook.trigger(ip);
  if (response == 200){
    Serial.println("OK");
  }
  else{
    Serial.println("Failed");
  }
}

/* ------------------- SERVER -------------------  */

void handleRoot() {                         // When URI / is requested, send a web page with a button to toggle the mail button
  server.send(200, "text/html", "<html><title>Internet of Things - Demonstration</title><meta charset=\"utf-8\" \/> \
      </head><body><h1>Mail Notifier</h1> \
      <p>Click on the following button to collect your mail. You have 10 minutes before the device will be activated again</p> \
      <form action=\"/DELAY\" method=\"POST\" ><input type=\"submit\" value=\"Collect Mail\" style=\"width:500pxqco; height:100px; font-size:24px\"></form> \
      </body></html>");
}
void sendDelay() {                          // If a POST request is made to URI /LED
  Serial.println("You have 10 min. to collect your mail");
  want_collect = 1;
  server.sendHeader("Location", "/");       // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
  // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void serverSetup() {
  delay(10);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
 // wm.resetSettings();

  //default custom static IP
  /*char static_ip[16] = "192.168.0.115";
    char static_gw[16] = "10.0.1.1";
    char static_sn[16] = "255.255.255.0";
    // Sets static IP Address
    IPAddress _ip, _gw, _sn;
    _ip.fromString(static_ip);
    _gw.fromString(static_gw);
    _sn.fromString(static_sn);
  */
  IPAddress _ip = IPAddress(192, 168, 0, 115);
  IPAddress _gw = IPAddress(10, 0, 1, 1);
  IPAddress _sn = IPAddress(255, 255, 255, 0);

  wm.setSTAStaticIPConfig(_ip, _gw, _sn);

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AutoConnectAP"); // password protected ap

  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    Serial.print(WiFi.localIP());
  }

  if (MDNS.begin("iot")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/DELAY", HTTP_POST, sendDelay);
  server.onNotFound(handleNotFound);

  // Start the server
  server.begin();
  Serial.println("Server started");
}
