/* ************************************
 Read the time from RTC and display on OLED
 with an ESP8266<br> sda=0, scl=2
* *************************************/

// Libraries
#include <Wire.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "RTClib.h" //  Lady Ada
//Object declarations
RTC_DS1307 rtc;            // RTC
SSD1306  display(0x3c, 4, 5);//0x3C being the usual address of the OLED

//Month and Day Arrays. Put in Language of your choice, omitt the 'day' part of the weekdays
char *maand[] =
{
  "Januari", "Februari", "Maart", "April", "Mei", "Juni", "Juli", "Augustus", "September", "Oktober", "November", "December"
};
char *dagen[] = {"Zon", "Maan", "Dins", "Woens", "Donder", "Vrij", "Zater" };


// date and time variables
byte m = 0;          // contains the minutes, refreshed each loop
byte h = 0;          // contains the hours, refreshed each loop
byte s = 0;         // contains the seconds, refreshed each loop
byte mo = 0;       // contains the month, refreshes each loop
int j = 0;           // contains the year, refreshed each loop
byte d = 0;         // contains the day (1-31)
byte dag = 0;     // contains day of week (0-6)

void setup() {
  Serial.begin(9600);
  Wire.pins(4, 5);// yes, see text
  Wire.begin(4,5);// 0=sda, 2=scl
  rtc.begin();

// reading of time here only necessary if you want to use it in setup
  DateTime now = rtc.now();
  dag = now.dayOfTheWeek();
  j = now.year();
  mo = now.month();
  d = now.day();
  h = now.hour();
  m = now.minute();
  s = now.second();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime())
  {
    Serial.print(F("Current Unix time"));
    Serial.println(now.unixtime());
    Serial.print(F("Compiled Unix time"));
    Serial.println(compiled.unixtime());
    Serial.println("RTC is older than compile time! Updating");
    // following line sets the RTC to the date & time this sketch was compiled<br>   // uncomment to set the time
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialise the display.
  display.init();
  display.flipScreenVertically();// flipping came in handy for me with regard 
                                                                // to screen position
  display.setFont(ArialMT_Plain_10);

}

void loop() {
  display.clear();
  DateTime now = rtc.now();
  dag = now.dayOfTheWeek();
  j = now.year();
  mo = now.month();
  d = now.day();
  h = now.hour();
  m = now.minute();
  s = now.second();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  String t = String(h) + ":" + String(m) + ":" + String(s);
  String t2 = String(d) + ":"  + String(mo) + ":" + String(j);
  display.drawString(0, 0, t);//
  display.drawString(0, 14, t2);
  display.drawString(0, 28, maand[mo - 1]);
  String d = dagen[dag];
  d = d + "day";//adding the word 'dag' (=day)  to the names of the days
  display.drawString(0, 42, d);
  // write the buffer to the display
  display.display();
  delay(10);
}