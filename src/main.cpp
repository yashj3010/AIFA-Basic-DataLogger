
// ----------- LIBRARY INCLUDES ----------------
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>

// ----------- DEFINES ----------------
#define DHTPIN 2
#define DHTTYPE DHT11
#define LedYello 0
#define redPin 12
#define greenPin 13
#define bluePin 15
#define controllerID '1'
// ----------- VARIABLE DECLATIONS ----------------

// ----------- CONSTANT ----------------

const char *ssid = "Tenda_3681C0";
const char *password = "rjain1705";
const char *mqtt_server = "192.168.0.104";
const uint16_t kRecvPin = 15;

// ----------- LONG ----------------
long lastMsg = 0;
long duration, distance;

// ----------- CHAR ----------------
char lightchar[50];
char distanceChar[50];
char msg[50];

// ----------- INTEGERS ----------------

int value = 0;
int light;

// ----------- FLOATS ----------------

float temp = 0;

// ----------- ARRAYS ----------------

int outputPins[] = {0, 2, 4, 5, 12, 13, 15};

// ----------- INSTANCES ----------------

WiFiClient espClient;
PubSubClient client(espClient);
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
IRrecv irrecv(kRecvPin);
sensors_event_t event;
decode_results results;

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

int updatePinStatus(char Pin, int status)
{
  String Topic1 = "outTopic/" + (String)Pin + "status";
  
  client.publish((String)Topic1, PackFloatData(status, lightchar));

  
  return 0;
}
int getTemp()
{
  dht.humidity().getEvent(&event);
  Serial.print(F("Humidity: "));
  Serial.print(event.relative_humidity);
  Serial.println(F("%"));

  
  client.publish("outTopic/Humidity", PackFloatData(event.relative_humidity, lightchar));
  dht.temperature().getEvent(&event);
  client.publish("outTopic/Temp", PackFloatData(event.temperature, lightchar));
  return 0;
}

int getLight()
{
  light = analogRead(A0);
  Serial.print("Publish Light: ");
  Serial.println(light);
  client.publish("outTopic/Light", PackIntData(light, lightchar));
  return 0;
}

int getMoisture1()
{
  return 0;
}

int getMoisture2()
{
  return 0;
}

int getDateTime()
{
  return 0;
}

int logData()
{
  return 0;
}

int getIR()
{
  if (irrecv.decode(&results))
  {
    Serial.println("recoeved something");
    if (results.value == 0xC084)
    {
      if (digitalRead(5) == LOW)
      {
        digitalWrite(5, HIGH);
        Serial.println("bla");
      }
      else
      {
        digitalWrite(5, LOW);
        Serial.println("blaq");
      }
    }
    irrecv.resume();
  }
  return 0;
}

int setup_wifi()
{

  delay(10);
  // We start by connecti ng to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

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
    Serial.print("Attempting MQTT connection...");
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
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
    }
  }
}

// ----------- MQTT CALLBACK - RECIEVING VALUES  ----------------

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  if (strcmp(topic, "inTopic") == 0)
  {
    togglePins((String)payload[0]);
  }
}

void setup()
{

  // Starting Serial Monitor
  Serial.begin(115200);
  Serial.println("Started");

  // Sensor Init
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);

  delayMS = sensor.min_delay / 1000;
  irrecv.enableIRIn(); // Start the receiver
  Serial.println(kRecvPin);

  // CALLING FUNCTIONS
  setOutputMode(outputPins);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{

  if (!client.connected())
  {
    Serial.println("wifi Connected");
    reconnect();
  }

  client.loop();
  int before = 0;
  long now = millis();

  if (now - lastMsg > 1000)
  {

    lastMsg = now;
    getTemp();
    getLight();
    getMoisture1();
    getMoisture2();
    getDateTime();
    getIR();
  }
}