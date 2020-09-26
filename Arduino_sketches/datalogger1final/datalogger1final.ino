#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <virtuabotixRTC.h>
#include <SPI.h>
#include "SdFat.h"

#define DHTPIN 2
#define DHTTYPE    DHT11
#define LDRPIN A0
#define CPS A1
#define FILE_BASE_NAME "Day"
#define error(msg) sd.errorHalt(F(msg))
#define PowerLed 3
#define DataLed 4
#define ErrorLed 5

DHT_Unified dht(DHTPIN, DHTTYPE);
virtuabotixRTC myRTC(6, 7, 8);
SdFat sd;
SdFile file;

const int chipSelect = 10;
const uint32_t SAMPLE_INTERVAL_MS = 50000;
const uint8_t ANALOG_COUNT = 3;

uint32_t logTime;

void writeHeader() {
  digitalWrite(DataLed, HIGH);
  file.print(F("TimeStamp"));
  file.print(F(",Light"));
  file.print(F(",Moisture 1"));
  file.print(F(",Moisture 2"));
  file.print(F(",Temp"));
  file.print(F(",Humidity"));
  file.println();
  delay(250);
  digitalWrite(DataLed, LOW);
}

void logData() {
  uint16_t data[ANALOG_COUNT];
  digitalWrite(DataLed, HIGH);

  for (uint8_t i = 0; i < ANALOG_COUNT; i++) {
    data[i] = analogRead(i);
  }
  myRTC.updateTime();
  String hours = (String)myRTC.hours;
  String mins = (String)myRTC.minutes;
  String timestamp = hours + "." + mins;
  file.print((int)timestamp);

  for (uint8_t i = 0; i < ANALOG_COUNT; i++) {
    file.write(',');
    file.print(data[i]);
  }

  sensors_event_t event;
  dht.temperature().getEvent(&event);
  file.write(',');
  file.print(event.temperature);
  delay(250);
  dht.humidity().getEvent(&event);
  file.write(',');
  file.print(event.relative_humidity);
  file.println();
  delay(250);
  digitalWrite(DataLed, LOW);
}


void setup() {
  Serial.begin(9600);
  
  digitalWrite(PowerLed, HIGH);
  digitalWrite(ErrorLed, LOW);
  digitalWrite(DataLed, LOW);
  
  pinMode(ErrorLed , OUTPUT);
  pinMode(PowerLed , OUTPUT);
  pinMode(DataLed , OUTPUT);
  
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";


  dht.begin();

  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }
  delay(1000);

  Serial.println(F("Type any character to start"));
  while (!Serial.available()) {
    SysCall::yield();
  }

  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }

  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
    digitalWrite(ErrorLed, HIGH);
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
      digitalWrite(ErrorLed, HIGH);
    }
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    error("file.open");
    digitalWrite(ErrorLed, HIGH);
  }


  do {
    delay(10);
  } while (Serial.available() && Serial.read() >= 0);
  
  Serial.print(F("Logging to: "));
  Serial.println(fileName);
  Serial.println(F("Type any character to stop"));

  writeHeader();

}

void loop() {
  delay(1000);

  logData();
  myRTC.updateTime();

Serial.print(myRTC.minutes);
  if (myRTC.minutes == 8) {
    file.close();
    Serial.println(F("Done"));
    digitalWrite(ErrorLed, HIGH);
    digitalWrite(DataLed, HIGH);
    SysCall::halt();
    
  }
}
