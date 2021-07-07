
// ----------- LIBRARY INCLUDES ----------------
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include <Wire.h>
#include "SSD1306.h"
#include <ESP8266WiFi.h>          
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

// ----------- DEFINES ----------------
#define DHTPIN 0
#define DHTTYPE DHT11
#define controllerID '1'
#define analogInput A0
#define s0 14
#define s1 12
#define s2 13
#define s3 15
#define statusLed 2
// ----------- VARIABLE DECLATIONS ----------------

// ----------- CONSTANT ----------------
const char* ssid = "Tenda_3681C0";
const char* password = "rjain1705";
const char *mqtt_server = "192.168.0.101";

// ----------- LONG ----------------
long lastMsg = 0;

// ----------- CHAR ----------------
char lightchar[200];

// ----------- INTEGERS ----------------
int value = 0;
int light= 0;
int soilMoisture1 = 0;
int soilMoisture2 = 0;

// ----------- FLOATS ----------------

// ----------- STRING ----------------
String csvData;
String timeStamp;
String date;
String tempStr;
String humidityStr;
String SM1Str;
String SM2Str;
String lightStr;

// ----------- ARRAYS ----------------
/*
0  -- D3 -- Temp Sensor 
2  -- D4 -- Status Led
4  -- D2 -- SDA -- OLED -- RTC
5  -- D1 -- SCL -- OLED -- RTC
12 -- D6 -- S1 
13 -- D7 -- S2
14 -- D5 -- S0
15 -- D8 -- S3
*/
int outputPins[] = {0,2, 4, 5, 12, 13, 14, 15};
int controlPins[4] = {s0, s1, s2, s3};

// ----------- INSTANCES ----------------
WiFiClient espClient;
PubSubClient client(espClient);
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
sensors_event_t event;
RTC_DS1307 rtc;
SSD1306  display(0x3c, 4, 5);

// ----------- HELPER FUNCTIONS ----------------
char *PackIntData(int a, char b[])
{
  String pubString = String(a);
  pubString.toCharArray(b, pubString.length() + 1);
  return b;
}

char *PackFloatData(float a, char b[])
{
  String pubString = String(a);
  pubString.toCharArray(b, pubString.length() + 1);
  return b;
}
char *PackStringData(String a, char b[])
{
  String pubString = String(a);
  pubString.toCharArray(b, pubString.length() + 1);
  return b;
}

int setOutputMode(int outputPins[])
{
  int len = sizeof(outputPins) / sizeof(outputPins[0]);
  for (int i = 0; i < len ; i++)
  {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }
  return 0;
}
int updatePinStatus(char Pin, int status)
{
  client.publish("outTopic/pinStatus", PackIntData(status, lightchar));
  return 0;
}

int togglePins(String payload)
{
  if (payload[0] == controllerID){

    if (payload[2] == 'H'){
      digitalWrite(payload[1],HIGH);
      updatePinStatus((char)payload[1],1);
    }
    else if (payload[2] == 'L')
    {
      digitalWrite(payload[1],LOW);
      updatePinStatus((char)payload[1],0);
    }
  }
  return 0;
}

int getTemp()
{
  digitalWrite(statusLed, HIGH);
  dht.humidity().getEvent(&event);
  Serial.print(F("Humidity: "));
  Serial.print(event.relative_humidity);
  Serial.print("\n");
  client.publish("outTopic/Humidity", PackFloatData(event.relative_humidity, lightchar));
  humidityStr = String(event.relative_humidity);

  dht.temperature().getEvent(&event);
  Serial.print(F("temperature: "));
  Serial.print(event.temperature);
  Serial.print("\n");
  client.publish("outTopic/Temp", PackFloatData(event.temperature, lightchar));
  tempStr = String(event.temperature);

  return 0;
}

int getDateTime()
{
 DateTime time = rtc.now();
 date = String(time.timestamp(DateTime::TIMESTAMP_DATE));
 timeStamp = String(time.timestamp(DateTime::TIMESTAMP_TIME));
 Serial.print(timeStamp);
 Serial.print(date);
 Serial.print("\n");


 client.publish("outTopic/Date", PackStringData(date, lightchar));
 client.publish("outTopic/Time", PackStringData(timeStamp, lightchar));
}

void getMoisture1(){
    digitalWrite(s0, LOW);
    digitalWrite(s1, LOW);

    soilMoisture1 = analogRead(analogInput);
    SM1Str = (String)soilMoisture1;
    Serial.print("Soil Moisture 1:");
    Serial.print(soilMoisture1);
    Serial.print("\n");
    client.publish("outTopic/SM1", PackStringData(SM1Str, lightchar));
}

void getMoisture2(){
    digitalWrite(s0, HIGH);
    digitalWrite(s1, LOW);

    soilMoisture2 = analogRead(analogInput);
    SM2Str = (String)soilMoisture2;
    Serial.print("Soil Moisture 2:");
    Serial.print(soilMoisture2);
    Serial.print("\n");
    client.publish("outTopic/SM2", PackStringData(SM2Str, lightchar));
}
void getLight(){
    digitalWrite(s0, LOW);
    digitalWrite(s1, HIGH);

    light = analogRead(analogInput);
    lightStr = (String)light;
    Serial.print("Light:");
    Serial.print(light);
    Serial.print("\n");
    client.publish("outTopic/Light", PackStringData(lightStr, lightchar));
}

int displayOled()
{
  // csvData = date+ "," + timeStamp + "," + tempStr + "," + humidityStr + "," + SM1Str + "," + SM2Str + "," + lightStr;
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);

  display.drawString(0, 0,date);
  display.drawString(0, 14, timeStamp);

  display.drawString(0, 28, "temp:");
  display.drawString(63, 28, tempStr);

  display.drawString(0, 41, "humidity: ");
  display.drawString(63, 41, humidityStr);

  display.display();
  delay(2000);
  display.clear();
  display.drawString(0, 0,"SM1:");
  display.drawString(63, 0,SM1Str);

  display.drawString(0, 14, "SM2:");
  display.drawString(63, 14, SM2Str);

  display.drawString(0, 28, "Light:");
  display.drawString(63, 28, lightStr);
  display.display();
  delay(2000);

  return 0;
}
int logData()
{
  csvData = (String)controllerID + "," + date + "," + timeStamp + "," + tempStr + "," + humidityStr + "," + SM1Str + "," + SM2Str + "," + lightStr;
  Serial.print("csvData:");
  Serial.print(csvData);
  Serial.print("\n");
  client.publish("outTopic/csvData", PackStringData(csvData, lightchar));
  digitalWrite(statusLed, LOW);
  return 0;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// ----------- MQTT CALLBACK - RECIEVING VALUES  ----------------

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Message arrived [");
  Serial.println(topic);
  Serial.print("] ");
  for (int i = 0; i < (int)length; i++)
  {
    Serial.println((char)payload[i]);
  }

  if (strcmp(topic, "inTopic") == 0)
  {
    //togglePins((String)payload[0]);
    Serial.println((String)payload[0]);

  }
}

void setup()
{
  Serial.begin(9600);

  display.init();
  display.clear();
  display.flipScreenVertically();// flipping came in handy for me with regard 
  display.setFont(ArialMT_Plain_16);

  display.clear();
  display.drawString(0, 0, "Init Complete");
  display.display();
  display.clear();

  display.drawString(0, 0, "Welcome");
  display.drawString(0, 14, "Initialising");
  display.display();

  display.clear();
  display.drawString(0, 0, "Webserver");
  display.drawString(0, 14, "Initialised");
  display.display();

  WiFiManager wifiManager;
  wifiManager.autoConnect("Smart Irrigation Node 1");
  Serial.println("connected...yeey :");

  display.clear();
  display.drawString(0, 0, "connected to");
  display.drawString(0, 14, "Wifi");
  display.display();

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  pinMode(analogInput, INPUT);

  Wire.pins(4, 5);// 4=sda, 5=scl
  Wire.begin(4,5);// 4=sda, 5=scl

  rtc.begin();
  Serial.println("Started");
   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
    display.clear();
    display.drawString(0, 0, "Error");
    display.drawString(0, 14, "RTC 404");
    display.display();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Sensor Init
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);

  delayMS = sensor.min_delay / 1000;
  
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("Connecting to WIFI");
  display.clear();
  display.drawString(0, 0, "Connecting");
  display.drawString(0, 14, "WIFI");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  display.clear();
  display.drawString(0, 0, "WiFi connected");
  display.display();

  // CALLING FUNCTIONS
  setOutputMode(outputPins);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  display.clear();
  display.drawString(0, 0, "Server Connected");
  display.display();
  delay(2000);
  client.setCallback(callback);
  display.clear();
  display.drawString(0, 0, "Callback Established");
  display.display();
}

void loop()
{
  delay(10);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();

  if (now - lastMsg > 2000)
  {
    lastMsg = now;

    getMoisture2();
    getMoisture1();
    getTemp();
    getDateTime();
    getLight();

    logData();
    
    displayOled();
  }
  else{
    digitalWrite(statusLed, HIGH);
    displayOled();
    delay(1000);
    digitalWrite(statusLed, LOW);
  }
}