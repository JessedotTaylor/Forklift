#define analogPin A7
//int analogPin = A0;
int val, val2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  val = analogRead(analogPin);
  val2 = val / 1024;
  //if (val != 0) {
    Serial.println(val);
  //}
  delay(500);
}
