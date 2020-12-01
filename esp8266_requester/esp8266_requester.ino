#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "SSD1306.h"

extern "C" {
#include "user_interface.h"
}

// Fill in your WiFi router SSID and password
const char* ssid = "<ssid>";
const char* password = "<password>";
const char* host = "<victim ip>";
const uint16_t port = 80;

//Buttons set up
int buttonUppin = 16; //D0
int buttonDownpin = 14; //D5

/* create display(Adr, SDA-pin, SCL-pin) */
SSD1306 display(0x3c, 5, 4);   // GPIO 5 = D1, GPIO 4 = D2

#define flipDisplay true;
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
int curLevel = 0;
String url = "/ledoff";

void setup() {

  // button setup
  pinMode(buttonUppin, INPUT_PULLUP);
  pinMode(buttonDownpin, INPUT_PULLUP);
  
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
  display.drawString(0, 50, "Scada Requester");
  display.display();
  delay(2500);
  display.clear();

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
  display.display();
  delay(10000);
  display.clear();
  display.display();

}

void changlevel(int up) {
  display.clear();
  display.drawString(0, 0, "Connecting to ");
  display.drawString(0, 10, (String)host);
  display.display();

  if (up == 1) {
    if (curLevel == 0) {
      url = "/led25";
      curLevel++;
    }
    else if (curLevel == 1) {
      url = "/led50";
      curLevel++;
    }
    else if (curLevel == 2) {
      url = "/led75";
      curLevel++;
    }
    else if (curLevel == 3) {
      url = "/led100";
      curLevel++;
    } else if (curLevel == 4) {}
  } else {
    if (curLevel == 4) {
      url = "/led75";
      curLevel--;
    } else if (curLevel == 3) {
      url = "/led50";
      curLevel--;
    } else if (curLevel == 2) {
      url = "/led25";
      curLevel--;
    } else if (curLevel == 1) {
      url = "/ledoff";
      curLevel--;
    } else if (curLevel == 0) {}
  }
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    display.drawString(0, 20, "Connection failed");
    display.display();
    delay(5000);
    display.clear();
    return;
  }

  // This will send a string to the server
  display.drawString(0, 20, "Sending data to server");
  display.display();
  if (client.connected()) {
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" + 
                "Connection: close\r\n\r\n");
    display.drawString(0, 30, "Request sent:");
    display.drawString(0, 40, (String)url);
    display.display();
    delay(10000);
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      display.drawString(0, 50, ">>> Client Timeout !");
      display.display();
      client.stop();
      delay(5000);
      display.clear();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  display.clear();
  display.drawString(0, 0, "Receiving from server");
  display.display();
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (line == "HTTP/1.1 200 OK") {
      display.drawString(0, 20, "200 OK");
      break;
    } else {
      display.drawString(0, 20, "Request Failed");
      break;
    }
    display.display();
  }

  // Close the connection
  display.drawString(0, 40, "Closing the connection");
  display.display();
  delay(10000);
  display.clear();
  display.display();
  client.stop();
}

void loop() {

  if (digitalRead(buttonUppin) == LOW){
    changlevel(1);
  }
  if (digitalRead(buttonDownpin) == LOW){
    changlevel(0);
  }

}
