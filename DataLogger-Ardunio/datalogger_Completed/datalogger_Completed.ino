
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

// ----------- DEFINES ----------------
#define DHTPIN 0
#define DHTTYPE DHT11
#define controllerID '1'
#define analogInput A0
#define s0 14
#define s1 12
#define statusLed 2
// ----------- VARIABLE DECLATIONS ----------------

// ----------- CONSTANT ----------------
const int airValue1 = 986;
const int airValue2 = 914;

const int waterValue1 = 537;
const int waterValue2 = 526;

const char *ssid = "Node 0";
const char *password = "yashj1030";
const char *mqtt_server = "192.168.0.113";

// ----------- LONG ----------------
long lastMsg = 0;


// ----------- CHAR ----------------
char lightchar[200];

// ----------- INTEGERS ----------------

int value = 0;
int light = 0;
int lightpercent = 0;
int soilMoisture1 = 0;
int soilMoisture2 = 0;
int soilmoisturepercent1 = 0;
int soilmoisturepercent2 = 0;
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
int outputPins[] = {0, 2, 4, 5, 12, 13, 14, 15};

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

void getTemp()
{
  //  digitalWrite(statusLed, HIGH);
  dht.humidity().getEvent(&event);
  //  Serial.print(F("Humidity: "));
  //  Serial.print(event.relative_humidity);
  //  Serial.print("\n");
  client.publish("outTopic/Humidity", PackFloatData(event.relative_humidity, lightchar));
  humidityStr = String(event.relative_humidity);

  //  dht.temperature().getEvent(&event);
  //  Serial.print(F("temperature: "));
  //  Serial.print(event.temperature);
  //  Serial.print("\n");
  client.publish("outTopic/Temp", PackFloatData(event.temperature, lightchar));
  tempStr = String(event.temperature);
}

void getDateTime()
{
  DateTime time = rtc.now();
  date = String(time.timestamp(DateTime::TIMESTAMP_DATE));
  timeStamp = String(time.timestamp(DateTime::TIMESTAMP_TIME));
  // Serial.print(timeStamp);
  // Serial.print(date);
  // Serial.print("\n");


  client.publish("outTopic/Date", PackStringData(date, lightchar));
  client.publish("outTopic/Time", PackStringData(timeStamp, lightchar));
}

void getMoisture1() {
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  soilMoisture1 = analogRead(analogInput);
  
  soilmoisturepercent1 = map(soilMoisture1, airValue1, waterValue1, 0, 100);
  
  if (soilmoisturepercent1 >= 100)
  {
    soilmoisturepercent1 = 100;
  }
  else if (soilmoisturepercent1 <= 0)
  {
    soilmoisturepercent1 = 0;
  }
    SM1Str = (String)soilmoisturepercent1;
  client.publish("outTopic/SM1", PackStringData(SM1Str, lightchar));
}

void getMoisture2() {
  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);
  soilMoisture2 = analogRead(analogInput);
  soilmoisturepercent2 = map(soilMoisture2, airValue2, waterValue2, 0, 100);
  
  if (soilmoisturepercent2 >= 100)
  {
    soilmoisturepercent2 = 100;
  }
  else if (soilmoisturepercent2 <= 0)
  {
    soilmoisturepercent2 = 0;
  }
    SM2Str = (String)soilmoisturepercent2;
  client.publish("outTopic/SM2", PackStringData(SM2Str, lightchar));
}
void getLight() {
  digitalWrite(s0, LOW);
  digitalWrite(s1, HIGH);
  light = analogRead(analogInput);
  lightpercent = map(light, 0, 1024, 0, 100);
  lightStr = (String)lightpercent;
  //    Serial.print("Light:");
  //    Serial.print(light);
  //    Serial.print("\n");
  client.publish("outTopic/Light", PackStringData(lightStr, lightchar));
}

void displayOled()
{
  // csvData = date+ "," + timeStamp + "," + tempStr + "," + humidityStr + "," + SM1Str + "," + SM2Str + "," + lightStr;
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);

  display.drawString(0, 0, date); //
  display.drawString(0, 14, timeStamp);

  display.drawString(0, 28, "tempe:");
  display.drawString(63, 28, tempStr);

  display.drawString(0, 41, "humidity: ");
  display.drawString(63, 41, humidityStr);

  display.display();
  delay(2000);
  display.clear();

  display.drawString(0, 0, "SM1:");
  display.drawString(63, 0, SM1Str);

  display.drawString(0, 14, "SM2:");
  display.drawString(63, 14, SM2Str);

  display.drawString(0, 28, "Light:");
  display.drawString(63, 28, lightStr);
  display.display();
  delay(2000);
}

void logData()
{
  getMoisture2();
  getMoisture1();
  getTemp();
  getDateTime();
  getLight();

  csvData = date + "," + timeStamp + "," + tempStr + "," + humidityStr + "," + SM1Str + "," + SM2Str + "," + lightStr  +"," +  "2";
  Serial.print("csvData:");
  Serial.print(csvData);
  Serial.print("\n");
  client.publish("outTopic/csvData", PackStringData(csvData, lightchar));
  displayOled();
  digitalWrite(statusLed, LOW);
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
    Serial.print("Something Arrived");
  }
}

void setup()
{
  display.init();
  display.flipScreenVertically();// flipping came in handy for me with regard
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Init Complete");
  display.display();
  display.clear();

  display.drawString(0, 0, "Welcome");
  display.drawString(0, 14, "Initialising");
  display.display();

  display.clear();
  display.drawString(0, 0, "Autoconnect");
  display.drawString(0, 14, "Initialised");
  display.display();

  display.clear();
  display.drawString(0, 0, "connected to");
  display.drawString(0, 14, "Wifi");
  display.display();

  Serial.begin(9600);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(analogInput, INPUT);

  Wire.pins(4, 5);// 4=sda, 5=scl
  Wire.begin(4, 5); // 4=sda, 5=scl
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
  Serial.println("Connecting to ");
  Serial.println(ssid);
  display.clear();
  display.drawString(0, 0, "Connecting");
  display.drawString(0, 14, (String)ssid);
  display.display();
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
  display.clear();
  display.drawString(0, 0, "WiFi connected");
  display.display();

  // CALLING FUNCTIONS
  setOutputMode(outputPins);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


}

void loop()
{
  delay(10);
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
  else {
    digitalWrite(statusLed, HIGH);
    displayOled();
    delay(1000);
    digitalWrite(statusLed, LOW);
  }
}
