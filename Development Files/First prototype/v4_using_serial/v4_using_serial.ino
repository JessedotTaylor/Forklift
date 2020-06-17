//Define motor pins
#define PenA 11    //Code expecting left hand motor
#define Pin1 6
#define Pin2 5
#define PenB 10    //Code expecting right hand motor
#define Pin3 4
#define Pin4 3

int getDur(char * p);
void inpToAction(void);


char c = ' ';
int data = 0;
int dur;


int serialCount, readyFlag = 0;
char incoming[80] = {'\0'};
bool waiting = false;

//void checkSerial() {
//  if (Serial.available() && !waiting) {
//    waiting = true;
//  }
//}
//
//void serialReady() {
//    Serial.write("Read Called\n", 12);
//    int x = 0;
//    while(Serial.available()){
//      incoming[x++] = Serial.read();
//    }
//    Serial.print(incoming);
//    readyFlag = 0x01;
//    Serial.flush();
//}

class DCMotor {
  public:
    DCMotor(int IN1, int IN2, int EN):in1(IN1), in2(IN2), en(EN) {
      pinMode(en, OUTPUT);
      pinMode(in1, OUTPUT);
      pinMode(in2, OUTPUT);
      
    // Set initial rotation direction
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
    }
    
    void lowOff() {digitalWrite(en, LOW);}
    void ToggleDir() {digitalWrite(in1, (!dir)); digitalWrite(in2,dir); dir = (!dir);}
    void ToggleState() {digitalWrite(en, (!s)); s = !s;}
     
    private:
      bool dir = true;
      int in1, in2, en;
      bool s = 0;
};

class MotorBus {
  public:
    MotorBus(DCMotor motor1, DCMotor motor2): m1(motor1), m2(motor2), movEn(false) {}

    void highOff() {
      m1.lowOff();
      m2.lowOff();
    }
  
    void motorForward(int Inpdur) {
      m1.ToggleState();
      m2.ToggleState();
      movEn = true;
      dur = Inpdur * 100000;
      previousMillis = millis();
    }
    
    void motorBack(int dur) {
      m1.ToggleDir();
      m2.ToggleDir();
      motorForward(dur);
      m1.ToggleDir();
      m2.ToggleDir();
    }
    
    void motorDir(int left, int dur) {
      if (left) {
        m1.ToggleDir();
      } else {
        m2.ToggleDir();
      }
      motorForward(dur);
      if (left) {
        m1.ToggleDir();
      } else {
        m2.ToggleDir();
      }
    }

    void clkCheck(unsigned long currentMillis) {
      //Serial.println(movEn);
      if((currentMillis - previousMillis >= dur) && movEn) {
        Serial.println("clk passed");
        m1.ToggleState();
        m2.ToggleState();
        //previousMillis = currentMillis;
        movEn = false;
      }
    }

    void d90(int left) {
      const int turnTime = 90;
      motorDir(left, turnTime);
    }

  private:
    DCMotor m1, m2;
    bool movEn;
    int dur;
    unsigned long previousMillis;  
};

int checkHall(void) {
  int returns[4] = {0,0,0,0};
  for (int x = 0; x <= 4; x++) {
    returns[x] = map(analogRead(x), 0, 1023, 0, 1);
  }
  if (returns[1] && returns[2]) {
    return 1; // A-OK, Straight line
  } else if (returns[0]) {
    return 2; //Left Junction
  } else if (returns[3]) {
    return 3; // Right Junction
  } else if (returns[0] && returns[3]) {
    return 4; //T junction
  } else {
    return 0;
  }
};
void setup() {
  // Setup motors
  

  Serial.begin(9600);

  //Interupt Setup - 1 per millisecond?
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

DCMotor motorA(Pin1, Pin2, PenA);
DCMotor motorB(Pin3, Pin4, PenB);

MotorBus Motors(motorA, motorB);

SIGNAL(TIMER0_COMPA_vect) {
  unsigned long currentMillis = millis();
  Motors.clkCheck(currentMillis);
  
  //checkSerial();
  
//  if ((serialCount++ >= 100) && waiting) {
//    serialReady();
//    serialCount = 0;
//    waiting = false;
//  }

  readyFlag = checkHall() << 4;
}

void loop() {
/*---------------------------------------------------------------------------------------- 
`available commands:
F X Forward for x milliseconds
B X Backward for x milliseconds
L X Left for x milliseconds
R X Right for x milliseconds
U X Up for x turns (on stepper)
D X Down for x turns (on stepper)

*/
if (Serial.available()){
  delay(500);
  int x = 0;
  while (Serial.available()) {
    incoming[x++] = Serial.read();
  }
  Serial.print(incoming);
  //Serial.flush();
  inpToAction();
}


//  if (readyFlag & 0x01) {
//    Serial.write("Ready Flag Checked", 18);
//    inpToAction();
//    readyFlag = readyFlag & !(0x01);  // Clear Serial Ready Flag
//  }
//  if (readyFlag >> 4) {
//    int retVal = readyFlag >> 4;
//    if (retVal == 2) {
//      //Left Junction
//      Serial.write("Left", 4);
//    } else if (retVal == 3) {
//      //Right Junction
//      Serial.write("Right", 5);
//    } else if (retVal == 4) {
//      //T Junction
//      Serial.write("T", 1);
//    }
//
//    readyFlag = readyFlag & !(0xF0);  //Clear Hall Effect Flag
//  }
}



int getDur(char * p) {
  int x = 1;
  int ret = 0;
  while (p[x] != '\0') {
    if ((p[x] - '0') >= 0 && (p[x] - '0') <= 9) {
      ret = (p[x] - '0') + (ret * 10);
    }
    x++;
  }
  //Serial.print(ret);
  return ret;
  
}

void inpToAction(void) {
  Serial.write("Switch called",14);
  switch (incoming[0])
    {
      case ('F' | 'f'):
        Serial.println("Forward called");
        dur = getDur(incoming);
        //Serial.print(dur);
        Motors.motorForward(dur);
        break;
  
      case ('B' | 'b'):
        Serial.println("Backward called");
        dur = getDur(incoming);
        Motors.motorBack(dur);
        break;
  
      case ('L' | 'l'):
        Serial.println("Left called");
        dur = getDur(incoming);
        Motors.motorDir(1,dur);
        break;
  
      case ('R'| 'r'):
        Serial.println("Right called");
        dur = getDur(incoming);
        Motors.motorDir(0,dur);
        break;

     case ('S' | 's'):
        Serial.println("Stop called");
        Motors.highOff();
        break;
  
      case ('U' | 'u'):
        Serial.write("Unimplemented Command\n");
        break;
  
      case ('D' | 'd'):
        Serial.write("Unimplemented Command\n");
        break;
  
     case ('\n'):
        break;
        
     default:
        Serial.write("Unknown Command\n");
        break;
    }
}



