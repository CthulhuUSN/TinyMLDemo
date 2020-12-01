#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "SSD1306.h"

extern "C" {
#include "user_interface.h"
}

// Fill in your WiFi router SSID and password
const char* ssid = "<SSID>";
const char* password = "<Password>";
MDNSResponder mdns;

ESP8266WebServer server(80);

/* create display(Adr, SDA-pin, SCL-pin) */
SSD1306 display(0x3c, 5, 4);   // GPIO 5 = D1, GPIO 4 = D2

#define flipDisplay true
/* Display settings */
#define minRow       0              /* default =   0 */
#define maxRow     127              /* default = 127 */
#define minLine      0              /* default =   0 */
#define maxLine     63              /* default =  63 */

/* render settings */
#define Row1         0
#define Row2        30
#define Row3        35
#define Row4        80
#define Row5        85
#define Row6       125

#define LineText     0
#define Line        12
#define LineVal     47

//===== Run-Time variables =====//
unsigned long prevTime   = 0;
unsigned long curTime    = 0;
unsigned long maxVal     = 0;
double multiplicator     = 0.0;
unsigned int val[128];
int curLevel = 0;
int prevLevel = 0;
int curLevelTime = 0;
int numRequests = 0;
String uartString = "";

const char INDEX_HTML[] =
  "<!DOCTYPE HTML>"
  "<html>"
  "<head>"
  "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
  "<title>ESP8266 IOT Demo</title>"
  "<style>"
  "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
  "</style>"
  "</head>"
  "<body>"
  "<h1>ESP8266 IOT Demo</h1>"
  "<FORM action=\"/\" method=\"post\">"
  "<P>"
  "LED<br>"
  "<INPUT type=\"radio\" name=\"LED\" value=\"0\">Off<BR>"
  "<INPUT type=\"radio\" name=\"LED\" value=\"1\">25%<BR>"
  "<INPUT type=\"radio\" name=\"LED\" value=\"2\">50%<BR>"
  "<INPUT type=\"radio\" name=\"LED\" value=\"3\">75%<BR>"
  "<INPUT type=\"radio\" name=\"LED\" value=\"4\">100%<BR>"
  "<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
  "</P>"
  "</FORM>"
  "</body>"
  "</html>";

// GPIO16---D0?
const int LEDPIN = 16;

void sendUartData(){
  Serial.print(curLevel);
  Serial.print(prevLevel);
  Serial.print(curLevelTime);
  Serial.print("!");
}

void getMultiplicator() {
  maxVal = 1;
  for (int i = 0; i < maxRow; i++) {
    if (val[i] > maxVal) maxVal = val[i];
  }
  if (maxVal > LineVal) multiplicator = (double)LineVal / (double)maxVal;
  else multiplicator = 1;
}

void handleRoot()
{
  if (server.hasArg("LED")) {
    handleSubmit();
    numRequests++;
    sendUartData();
  }
  else {
    server.send(200, "text/html", INDEX_HTML);
    numRequests++;
  }
}

void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}

void handleSubmit()
{
  String LEDvalue;

  if (!server.hasArg("LED")) return returnFail("BAD ARGS");
  LEDvalue = server.arg("LED");
  if (LEDvalue == "0") {
    writeLED(0);
    curLevel = 0;
    server.send(200, "text/html", INDEX_HTML);
  }
  else if (LEDvalue == "1") {
    writeLED(256);
    curLevel = 25;
    server.send(200, "text/html", INDEX_HTML);
  }
  else if (LEDvalue == "2") {
    writeLED(512);
    curLevel = 50;
    server.send(200, "text/html", INDEX_HTML);
  }
  else if (LEDvalue == "3") {
    writeLED(768);
    curLevel = 75;
    server.send(200, "text/html", INDEX_HTML);
  }
  else if (LEDvalue == "4") {
    writeLED(1023);
    curLevel = 100;
    server.send(200, "text/html", INDEX_HTML);
  }
  else {
    returnFail("Bad LED value");
  }
}

void returnOK()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK\r\n");
}

/*
   Imperative to turn the LED off using a non-browser http client.
   For example, using wget.
   $ wget http://esp8266webform/ledoff
*/
void handleLEDoff()
{
  writeLED(0);
  curLevel = 0;
  returnOK();
  numRequests++;
  sendUartData();
}

/*
   Imperative to turn the LED on using a non-browser http client.
   For example, using wget.
   $ wget http://esp8266webform/led25
*/
void handleLED25()
{
  writeLED(256);
  curLevel = 25;
  returnOK();
  numRequests++;
  sendUartData();
}

/*
   Imperative to turn the LED on using a non-browser http client.
   For example, using wget.
   $ wget http://esp8266webform/led50
*/
void handleLED50()
{
  writeLED(512);
  curLevel = 50;
  returnOK();
  numRequests++;
  sendUartData();
}

/*
   Imperative to turn the LED on using a non-browser http client.
   For example, using wget.
   $ wget http://esp8266webform/led75
*/
void handleLED75()
{
  writeLED(768);
  curLevel = 75;
  returnOK();
  numRequests++;
  sendUartData();
}

/*
   Imperative to turn the LED on using a non-browser http client.
   For example, using wget.
   $ wget http://esp8266webform/led100
*/
void handleLED100()
{
  writeLED(1023);
  curLevel = 100;
  returnOK();
  numRequests++;
  sendUartData();
}


void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  numRequests++;
}

void writeLED(int LEDLevel)
{
  analogWrite(LEDPIN, LEDLevel);
}

void setup(void)
{

  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);
  writeLED(1023);

  /* start Display */
  display.init();
  if (flipDisplay) display.flipScreenVertically();

  /* show start screen */
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "TinyML-");
  display.drawString(0, 16, "Demo");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 40, "Version 1.1");
  display.drawString(0, 50, "Scada Victim");
  display.display();
  delay(2500);
  display.clear();
  writeLED(0);

  WiFi.begin(ssid, password);

  display.drawString(0, 0, "Connecting ...");
  display.display();
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
  }
  display.clear();
  display.drawString(0, 10, "Connected");
  display.drawString(0, 20, "SSID:");
  display.drawString(32, 20, (String)WiFi.SSID());
  display.drawString(0, 30, "IP:");
  display.drawString(32, 30, WiFi.localIP().toString());

  if (mdns.begin("esp8266WebForm", WiFi.localIP())) {
    display.drawString(0, 40, "MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/ledoff", handleLEDoff);
  server.on("/led25", handleLED25);
  server.on("/led50", handleLED50);
  server.on("/led75", handleLED75);
  server.on("/led100", handleLED100);
  server.onNotFound(handleNotFound);

  server.begin();
  display.drawString(0, 50, "HTTP server started");
  display.display();
  delay(10000);
  display.clear();
  display.display();
}

void loop(void) {
  curTime = millis();
  server.handleClient();
  
  //every second
  if (curTime - prevTime >= 1000) {
    prevTime = curTime;
    
    if (curLevel == prevLevel) {
      curLevelTime++;
    } else {
      prevLevel = curLevel;
      curLevelTime = 0;
    }

    //move every bar one pixel to the left
    for (int i = 0; i < maxRow; i++) {
      val[i] = val[i + 1];
    }
    val[127] = curLevel;

    //recalculate scaling factor
    getMultiplicator();

    //draw display
    display.clear();
    display.drawLine(minRow, Line, maxRow, Line);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(Row1, LineText, "CL:");
    display.drawString(Row3, LineText, "CLT:");
    display.drawString(Row5, LineText, "Reqs:");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(Row2, LineText, (String)curLevel);
    display.drawString(Row4, LineText, (String)curLevelTime);
    display.drawString(Row6, LineText, (String)numRequests); // Need to add this into the requests section.
    for (int i = 0; i < maxRow; i++) display.drawLine(i, maxLine, i, maxLine - val[i]*multiplicator);
    display.display();
  }
}
