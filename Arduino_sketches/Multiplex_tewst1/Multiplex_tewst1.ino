#define s0 14
#define s1 12

#define analogInput A0

int soilMoisture1 = 0;
int soilMoisture2 = 0;

void setup() {
  Serial.begin(9600);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);

  pinMode(analogInput, INPUT);

}

void loop() {
  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);
  int SM2 = analogRead(A0);
  Serial.print(SM2);
  Serial.print("\n");
  Serial.print("\n");
//
  digitalWrite(s0, LOW);
  digitalWrite(s1, HIGH);
  int Light = analogRead(A0);
  Serial.print(Light);
  Serial.print("\n");
  Serial.print("\n");

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  int SM1 = analogRead(A0);
  Serial.print(SM1);
  Serial.print("\n");
  Serial.print("\n");
  


}
