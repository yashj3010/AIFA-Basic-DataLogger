#define s0 5
#define s1 4
#define s2 0
#define s3 2
#define analogInput A0

int soilMoisture1 = 0;
int soilMoisture2 = 0;

int pins[4] = {5, 4, 0, 2};
void getMoisture1() {
  for (int i = 0; i < 2; i++) { // count input pins
    if (i == 0) {
      for (int j = 0; j < 4; j++) { // setting control lines
        digitalWrite(pins[j], LOW);
      }
      soilMoisture1 = analogRead(analogInput);
        Serial.println("Soil Moisture 1:");
        Serial.println(soilMoisture1);
    }
    if (i == 1) {
      for (int k = 1; k < 4; k++) { // setting control lines
        digitalWrite(s0, HIGH);
        digitalWrite(pins[k], LOW);
      }
      soilMoisture2 = analogRead(analogInput);
        Serial.println("Soil Moisture 2:");
        Serial.println(soilMoisture2);
    }
    delay(500);
  }
}
  /*0
    void getMoisture2(){
    digitalWrite(s0, HIGH);
    digitalWrite(s1, LOW);
    digitalWrite(s2, LOW);
    digitalWrite(s3, LOW);
    soilMoisture2 = analogRead(analogInput);
    Serial.println("Soil Moisture 2:");
    Serial.println(soilMoisture2);
    }
  */
  void setup() {
    Serial.begin(9600);
    pinMode(s0, OUTPUT);
    pinMode(s1, OUTPUT);
    pinMode(s2, OUTPUT);
    pinMode(s3, OUTPUT);// put your setup code here, to run once:
    pinMode(analogInput, INPUT);
  }

  void loop() {
    // put your main code here, to run repeatedly:
    getMoisture1();
    delay(3000);

  }
