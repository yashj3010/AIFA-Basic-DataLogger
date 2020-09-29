
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

const char *ssid = "MQTT-Server";
const char *password = "mqtt@123";
const char *mqtt_server = "192.168.0.101";
const uint16_t kRecvPin = 15;

// ----------- LONG ----------------
long lastMsg = 0;
long duration, distance;

// ----------- CHAR ----------------
char lightchar[200];
char distanceChar[50];
char msg[50];

// ----------- INTEGERS ----------------

int value = 0;
int light= 0;
int soilMoisture1 = 0;
int soilMoisture2 = 0;

// ----------- FLOATS ----------------

float temporaary = 0;
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
  tempStr = String(event.relative_humidity);

  dht.temperature().getEvent(&event);
  Serial.print(F("temperature: "));
  Serial.print(event.temperature);
  Serial.print("\n");
  client.publish("outTopic/Temp", PackFloatData(event.temperature, lightchar));
  humidityStr = String(event.temperature);

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

void getMoisture2(){

    digitalWrite(s0, HIGH);
    digitalWrite(s1, LOW);
    digitalWrite(s2, LOW);
    digitalWrite(s3, LOW);

    soilMoisture2 = analogRead(analogInput);
    Serial.print("Soil Moisture 2:");
    Serial.print(soilMoisture2);
    Serial.print("\n");
    }
void getMoisture1(){
    digitalWrite(s0, LOW);
    digitalWrite(s1, LOW);
    digitalWrite(s2, LOW);
    digitalWrite(s3, LOW);
    soilMoisture1 = analogRead(analogInput);
    Serial.print("Soil Moisture 1:");
    Serial.print(soilMoisture1);
    Serial.print("\n");
    }

int logData()
{
  getTemp();
  getDateTime();

  csvData = date + timeStamp + tempStr + humidityStr + SM1Str + SM2Str + lightStr;
  Serial.print("csvData:");
  Serial.print(csvData);
  Serial.print("\n");
  client.publish("outTopic/csvData", PackStringData(csvData, lightchar));
  digitalWrite(statusLed, LOW);
  return 0;
}

int setup_wifi()
{

  delay(10);
  // We start by connecti ng to a WiFi network
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(".");

    return 0;
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  return 0;
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "reconnected");
      // ... and resubscribe
      client.subscribe("inTopic");
    }
    else
    {
      Serial.println("failed, rc=");
      Serial.println(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
    }
  }
}

// ----------- MQTT CALLBACK - RECIEVING VALUES  ----------------

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Message arrived [");
  Serial.println(topic);
  Serial.println("] ");
  for (int i = 0; i < (int)length; i++)
  {
    Serial.println((char)payload[i]);
  }

  if (strcmp(topic, "inTopic") == 0)
  {
    togglePins((String)payload[0]);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);// put your setup code here, to run once:
  pinMode(analogInput, INPUT);
  // Starting Serial Monitor
  //Serial.begin(115200);
  Serial.println("Started");
   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
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
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // CALLING FUNCTIONS
  setOutputMode(outputPins);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  getMoisture2();
  delay(5000);
  getMoisture1();

  if (!client.connected())
  {
    Serial.println("wifi Connected");
    reconnect();
  }

  client.loop();
  //int before = 0;
  long now = millis();

  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    logData();
  }
  else{
    digitalWrite(statusLed, HIGH);
    delay(1000);
    digitalWrite(statusLed, LOW);
  }
}