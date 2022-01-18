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

#include "sensor_read.h"

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h> // If you get an include error here: try to download from "Tools" -> "Manage Libraries..."

#define WIFI_SSID "GalxyAP" // *** This needs to be changed according to which wifi is available 
#define WIFI_PASSWORD "password17" // *** This needs to be changed according to which wifi is available 

#define SMTP_HOST "smtp.live.com"
#define SMTP_PORT 587

// The sign in credentials for email
#define AUTHOR_EMAIL "dtu34338-2022@hotmail.com"
#define AUTHOR_PASSWORD "a16jdlvk%%%0dkvg#)+++"

// Recipient's email
#define RECIPIENT_EMAIL "dtu34338-2022@hotmail.com" // *** Insert your email adress

// Pin definition on ESP8266
#define INPUT_D2 D2 // *** ESP32: replace "D2" with "2" 
#define INPUT_A0 A0 // *** ESP32: replace "A0" with "0" 

// Variables used to read sensor
const int analog_input = INPUT_A0;
const int digital_input = INPUT_D2;
const float analog_treshold = 0.20; //treshold in V to see if postal box flap is open
const float source_voltage = 3.3; //source input voltage
const float acdc_max = 1023.0; //maximum value obtained form AC/DC converter


// The SMTP Session object used for email sending 
SMTPSession smtp;

// Callback function to get the email sending status 
void smtpCallback(SMTP_Status status); 
void sendingEmail();

void setup() {
  Serial.begin(115200);
  pinMode(digital_input,INPUT_PULLUP);

  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

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
 static bool mail_trigger = 0;
 static bool *ptr_trigger = &mail_trigger;
 
 tilt_sensor = isTilted(digital_input);
 photo_sensor = lightDetection(analog_input, source_voltage, acdc_max);
 box_state = got_mail(ptr_tilt, ptr_photo, analog_treshold);
 mail_trigger = main_fun(&box_state, ptr_sw1, ptr_sw2, ptr_trigger);

 // Sending mail if the sensors are active
 if (mail_trigger == 1)
 {
  Serial.println("Send");
  *ptr_trigger = 0;
  sendingEmail(); 
 }
 delay(100);

}


/* ------------------- EMAIL -------------------  */

// Callback function to get the email sending status 
void smtpCallback(SMTP_Status status){
  // Print the current status 
  Serial.println(status.info());

  // Print the sending result 
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

void sendingEmail(){

  // Enable the debug via Serial port noen debug or 0, basic debug or 1
  smtp.debug(1);

  // Set the callback function to get the sending results 
  smtp.callback(smtpCallback);

  // Declare the session config data 
  ESP_Mail_Session session;

  // Set the session config 
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  // Declare the message class 
  SMTP_Message message;
  
  // Set the message headers 
  message.sender.name = "Mailbox";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "You've got mail!";
  message.addRecipient("User", RECIPIENT_EMAIL); // ***

  // Send HTML message
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Go and check your mailbox.</h1><p>- Sent from your mailbox</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  // Connect to server with the session config 
  if (!smtp.connect(&session))
    return;

  // Start sending Email and close the session 
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
   
}

/* ------------------- SERVER -------------------  */

// Create an instance of the server
WiFiserver server(80);


//default custom static IP
char static_ip[16] = "192.168.0.100";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

void handleRoot();
void sendDelay();
void handleNotFound();

void setup() {
  Serial.begin(115200);
  delay(10);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // put your setup code here, to run once:
  Serial.begin(115200);

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  //wm.resetSettings();

  // Sets static IP Address
  IPAddress _ip, _gw, _sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);

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

void loop() {
  // Check if a client has connected
  server.handleClient();
}

void handleRoot() {                         // When URI / is requested, send a web page with a button to toggle the mail button
  server.send(200, "text/html", "<html><title>Internet of Things - Demonstration</title><meta charset=\"utf-8\" \/> \
      </head><body><h1>Mail Notifier</h1> \
      <p>Click on the following button to collect your mail. You have 10 minutes before the device will be activated again</p> \
      <form action=\"/DELAY\" method=\"POST\" ><input type=\"submit\" value=\"Collect Mail\" style=\"width:500pxqco; height:100px; font-size:24px\"></form> \
      </body></html>");
}

void sendDelay() {                          // If a POST request is made to URI /LED
  Serial.println("You have 10 min. to collect your mail");      
  server.sendHeader("Location", "/");       // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
  // Send it back to the browser with an HTTP status 303 (See Other) to redirect

}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
