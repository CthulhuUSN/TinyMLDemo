#include "painlessMesh.h"
#include "SSD1306.h"

extern "C" {
#include "user_interface.h"
}


Scheduler userScheduler; 
painlessMesh  mesh;
int period = 1000;
unsigned long time_now = 0;

#define Button1 D5;
bool button1_status = 0;

#define   MESH_PREFIX     "TinyMLDemo"
#define   MESH_PASSWORD   "password1"
#define   MESH_PORT       5555

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

void sendMessage() ; 
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
// Reading Status of the Attack button
//TODO:Fix this to make more sense to the attack and display data to screen
  if (digitalRead(Button1) == HIGH)
    button1_status = !button1_status;
// Serializing in JSON Format 
  DynamicJsonDocument doc(1024);
  doc["Request"] = "100";
  String msg;
  serializeJson(doc, msg);
  mesh.sendBroadcast(msg);
  //Serial.println(msg);
  taskSendMessage.setInterval((TASK_SECOND * 1));
}

void receivedCallback(uint32_t from, String &msg){}

void newConnectionCallback(uint32_t nodeId) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "--> startHere:");
  display.drawString(0, 20, "New Connection");
  display.drawString(0, 40, "nodeId =");
  display.drawString(0, 50, nodeId);
  display.display();
  //wait approx [period] ms
  while(millis() &lt; time_now + period){}
  display.clear();
}

void changedConnectionCallback() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 20, "Changed connections");
  display.display();
  //wait approx [period] ms
  while(millis() &lt; time_now + period){}
  display.clear();
}

void nodeTimeAdjustedCallback(int32_t offset) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Adjusted time");
  display.drawString(0, 20, mesh.getNodeTime());
  display.drawString(0, 40, "Offset = ");
  display.drawString(0, 50, offset);
  display.display();
  //wait approx [period] ms
  while(millis() &lt; time_now + period){}
  display.clear();
}

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
  display.drawString(0, 40, "Version 2.0");
  display.drawString(0, 50, "Scada Attacker");
  display.display();
  delay(2500);
  display.clear();

  //mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}