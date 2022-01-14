#ifndef SERVER.H
#define SERVER.H
#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESP8266mDNS.h>        // Include the mDNS library
// Create an instance of the server
ESP8266WebServer server(80);
void handleRoot();
void sendDelay();
void handleNotFound();
void serverSetup();

#endif
