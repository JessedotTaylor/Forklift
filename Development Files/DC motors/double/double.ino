#define enA 11    //Code expecting left hand motor
#define in1 7
#define in2 6
#define enB 10    //Code expecting right hand motor
#define in3 5
#define in4 4
#define button 4
int val = 0;

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  //pinMode(button, INPUT);
  // Set initial rotation direction
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  //analogWrite(enA, 125);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(enA, 255);
  //digitalWrite(enB, HIGH);
  analogWrite(enB, 255);
  //digitalWrite(enA, HIGH);
}
