
#include <WiFiManager.h>
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESP8266mDNS.h>        // Include the mDNS library
// Create an instance of the server
ESP8266WebServer server(80);

const char* input_parameter2 = "input_integer";

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

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("AutoConnectAP"); // password protected ap

    if(!res) {
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
  server.on("/LED", HTTP_POST, sendDelay);
  server.onNotFound(handleNotFound);
    
  // Start the server
  server.begin();
  Serial.println("Server started"); 
}

void loop() {
  // Check if a client has connected
  server.handleClient();
}

void handleRoot() {                         // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<html><title>Internet of Things - Demonstration</title><meta charset=\"utf-8\" \/> \ 
      </head><body><h1>Mail Notifier</h1> \ 
      <p>Click on the following button to collect your mail. You have 10 minutes before the device will be activated again</p> \
      <form action=\"/LED\" method=\"POST\" ><input type=\"submit\" value=\"Collect Mail\" style=\"width:500pxqco; height:100px; font-size:24px\"></form> \
      <p>Insert photoresistor threshold <form action="/get"> Enter an integer: <input type="text" name="input_integer"><input type="submit" value="Submit"></p> \
      <p>Vil du vide mere: Kig på hjemmesiden for uddannelsen : <a href=\"www.dtu.dk/net\">Netværksteknologi og it</a></p> \
      
}

void sendDelay() {                          // If a POST request is made to URI /LED
  Serial.print("You ha1ve 10 min. to collect your mail");      // Change the state of the LED
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
  delay(60000);
  // Send it back to the browser with an HTTP status 303 (See Other) to redirect
  
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
  
