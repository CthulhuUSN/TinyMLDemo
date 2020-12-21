#include "painlessMesh.h"
#include "SSD1306.h"

extern "C" {
#include "user_interface.h"
}

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
int request = 0;
String uartString = "";
int period = 2500;
unsigned long time_now = 0;

// GPIO16---D0
const int LEDPIN = 16; 

Scheduler userScheduler; 
painlessMesh  mesh;

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

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

void writeLED(int LEDLevel)
{
  analogWrite(LEDPIN, LEDLevel);
}

void handleRequest(String request)
{
  if (request == "0") {
    writeLED(0);
    curLevel = 0;
    numRequests++;
    sendUartData();
  }
  else if (request == "25") {
    writeLED(256);
    curLevel = 25;
    numRequests++;
    sendUartData();
  }
  else if (request == "50") {
    writeLED(512);
    curLevel = 50;
    numRequests++;
    sendUartData();
  }
  else if (request == "75") {
    writeLED(768);
    curLevel = 75;
    numRequests++;
    sendUartData();
  }
  else if (request == "100") {
    writeLED(1023);
    curLevel = 100;
    numRequests++;
    sendUartData();
  }
  else {
    
  }
}

// Needed for painless library
void sendMessage(){}

void receivedCallback( uint32_t from, String &msg ) {
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "deserializeJson() failed: ");
    display.drawString(0, 20, error.c_str());
    display.display();
    //wait approx [period] ms
    while(millis() &lt; time_now + period){}
    display.clear();
  }
  request = doc["Request"];
  handleRequest(request);
} 

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
  display.drawString(0, 40, "Version 2.0");
  display.drawString(0, 50, "Scada Victim");
  display.display();
  //wait approx [period] ms
  while(millis() &lt; time_now + period){}
  display.clear();
  writeLED(0);

  //mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

}

void loop(void) {
  curTime = millis();
  // it will run the user scheduler as well
  mesh.update();
  
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
