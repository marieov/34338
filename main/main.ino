/*
 UPDATE THIS PART !!!!!!!!!!!!!
 Project work for course 34338 - Telecommunication programming projects with Arduino January 2022
 Work areas:
 Sensor part: Krzysztof Jan Pac
 Mail notificiation: Marie Øverby
 Server: Søren Qvist, Marius Larsen
 */

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

/* The sign in credentials */
#define AUTHOR_EMAIL "dtu34338-2022@hotmail.com"
#define AUTHOR_PASSWORD "a16jdlvk%%%0dkvg#)+++"

/* Recipient's email*/
#define RECIPIENT_EMAIL "dtu34338-2022@hotmail.com" // *** Insert your email adress

//pin definition on ESP8266
//VERY IMPORTANT PART !!!
#define INPUT_D2 D2 //ESP32: replace "D2" with "2" ***
#define INPUT_A0 A0 //ESP32: replace "A0" with "0" ***

const int analog_input = INPUT_A0;
const int digital_input = INPUT_D2;
const float analog_treshold = 0.20; //treshold in V to see if postal box flap is open
const float source_voltage = 3.3; //source input voltage
const float acdc_max = 1023.0; //maximum value obtained form AC/DC converter
const unsigned long collection_dealy = 30000; // 30s

/*reading from tilt sensor */
int isTilted(int digital_input);
/*reading from light sensor with conversion to [V]*/
float lightDetection(int analog_input, float source_voltage, float acdc_max);
/*checking the status of mailbox*/
bool got_mail(int *ptr_tilt, float *ptr_photo, float analog_treshold);
/*deciding wheter send a notification or not, basing on aquired information from previous functions*/
bool main_fun(bool *flap_state, bool *isOpen, bool *isClosed, bool *send_trigger);
/*timer,used for switching on/off notofication feature*/
void my_timer(unsigned long *time_value, bool *ptr_trigger);
/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status); 
/*sending notification to the user when there is a mail in the mailbox*/
void sendingEmail();

/* The SMTP Session object used for Email sending */
SMTPSession smtp;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(digital_input,INPUT_PULLUP);

/*** WiFi connection process ****/
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
 static bool mail_trigger = 0; // variable used to trigger notification feature
 static bool *ptr_trigger = &mail_trigger; // pointer to the above variable
 static unsigned long mytime = 0; // store the value of time that passed since timer has started
 static bool want_collect = 0; // store request from the server when user want to collect the mail
 
/*** Sensing process ***/
 tilt_sensor = isTilted(digital_input);
 photo_sensor = lightDetection(analog_input, source_voltage, acdc_max);
 box_state = got_mail(ptr_tilt, ptr_photo, analog_treshold);
 mail_trigger = main_fun(&box_state, ptr_sw1, ptr_sw2, ptr_trigger);

 if (*ptr_trigger == 1) // && want_collect == 0
 {
  //send email
  Serial.println("Send");
  *ptr_trigger = 0;
  sendingEmail(); 
 }
 delay(100);

//add function for button if it was pushed on the server
// ** add here**

 //check if user want to collect mail from the postal box 
 if(want_collect == 1)
  {
    my_timer(&mytime, &want_collect);
  }

}

//function definitions

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

bool main_fun(bool *flap_state, bool *isOpen, bool *isClosed, bool *send_trigger)
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
    *send_trigger = 1;
    *isOpen = 0;
  }
  return *send_trigger;
}

void my_timer(unsigned long *time_value, bool *ptr_trigger)
{
  *time_value = millis();

  if(*time_value >= collection_dealy)
  {
    Serial.println("Time has passed");
    *time_value = 0;
    *ptr_trigger = 0;
  }
}

// Used for sending email
/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
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

  /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
  */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;
  
  /* Set the message headers */
  message.sender.name = "Mailbox";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "You've got mail!";
  message.addRecipient("User", RECIPIENT_EMAIL); // ***

  /*Send HTML message*/
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Go and check your mailbox.</h1><p>- Sent from your mailbox</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
   
}
