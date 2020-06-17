const int HALL_COUNT = 4;

int checkHall(void) {
  int returns[HALL_COUNT] = {0,0,0};
  for (int x = 0; x < HALL_COUNT; x++) {
    returns[x] = map(analogRead(x), 0, 1023, 0, 8);
  }
  if ((returns[1] == 0) && (returns[2] == 0)) {
    return 1; // A-OK, Straight line
  } else if ((returns[0] == 0)) {
    return 2; //Left Junction
  } else if ((returns[3] == 0)) {
    return 3; // Right Junction
  } else if ((returns[0] == 0) && (returns[3] == 0)) {
    return 4; //T junction
  } else {
    return 0;
  }
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  ///*
  int returns[HALL_COUNT] = {0,0,0,0};
  for (int x = 0; x < HALL_COUNT; x++) {
    //returns[x] = map(analogRead(x), 0, 1023, 0, 8);
    //Serial.print(returns[x]);
    Serial.print(analogRead(x));
    Serial.print(' ');  
  }
  Serial.println();
  delay(100);
  //*/
  /*
  if (checkHall()){
    Serial.println(checkHall());
  }
  */
  delay(100);
  
}
