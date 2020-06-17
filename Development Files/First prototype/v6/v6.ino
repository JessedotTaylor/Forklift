//Define motor pins
#define PenA 11    //Code expecting left hand motor
#define Pin1 6
#define Pin2 5
#define PenB 10    //Code expecting right hand motor
#define Pin3 4
#define Pin4 3

int getDur(char * p);


char c = ' ';
int data = 0;
int dur;


int serialCount = 0;
int readyFlag = 0;
const int INCOMING_BUFFER = 10;
char incoming[INCOMING_BUFFER] = {'\0'};
volatile int waiting = 0;

void checkSerial() {
  if (Serial.available() && !waiting) {
    //Serial.println("Checked and available");
    waiting = 1;
  }
}

void serialReady(char * inc) {
    //Serial.println("Read Called");
    int x = 0;
    while(Serial.available()){
      incoming[x++] = Serial.read();
    }
    //Serial.println(incoming);
    readyFlag = 0x01;
    waiting = 0;
}

void nonIntRead(char * inc){
  if (Serial.available()) {
    Serial.println("Read Called");
    delay(100); 
    int x = 0;
    while(Serial.available()){
      inc[x++] = Serial.read();
      //Serial.print(c);
    }
    Serial.println(inc);
    Serial.println("Read Completed");
  }
}

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
    
    void lowOff() {digitalWrite(en, LOW); s = 0;}
    void ToggleDir() {digitalWrite(in1, (!dir)); digitalWrite(in2,dir); dir = (!dir);}
    void ToggleState() {digitalWrite(en, (!s)); s =!s;}
     
    private:
      bool dir = true;
      int in1, in2, en;
      bool s = 0;
};

class MotorBus {
  public:
    MotorBus(DCMotor motor1, DCMotor motor2): m1(motor1), m2(motor2), movEn(false), rev(false) {}

    void highOff() {
      m1.lowOff();
      m2.lowOff();
      movEn = false;
    }
  
    void motorForward(int Inpdur) {
      m1.ToggleState();
      m2.ToggleState();
      movEn = true;
      dur = Inpdur * 1000;
      previousMillis = millis();
    }
    
    void motorBack(int dur) {
      rev = true;
      toggleMotorDir();
      motorForward(dur);
      //rev = false;
    }

    void toggleMotorDir(void) {
      m1.ToggleDir();
      m2.ToggleDir();
    }
    
//    void motorDir(int left, int dur) {
//      if (left) {
//        m1.ToggleDir();
//      } else {
//        m2.ToggleDir();
//      }
//      motorForward(dur);
//      if (left) {
//        m1.ToggleDir();
//      } else {
//        m2.ToggleDir();
//      }
//    }

    void clkCheck(unsigned long currentMillis) {
      if((currentMillis - previousMillis >= dur) && movEn) {
        m1.ToggleState();
        m2.ToggleState();
        //previousMillis = currentMillis;
        movEn = false;

        if (rev) {
          toggleMotorDir();
          rev = false;
        }
      }
    }

//    void d90(int left) {
//      #define turnTime 90
//      motorDir(left, turnTime);
//      
//    }

  private:
    DCMotor m1, m2;
    bool movEn, rev;
    
    unsigned long previousMillis, dur;  
};

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
  
  checkSerial();
  //Serial.println(waiting);
  if ((serialCount++ >= 200) && (waiting == 1)) {
    //Serial.println("over 200");
    serialCount = 0;
    waiting = 2;
  } 
  if (serialCount > 200) {
    serialCount = 0;
  }

  //readyFlag = checkHall() << 4;
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
  if (waiting == 2) {
     serialReady(incoming);
  }
  //Serial.println(readyFlag + '0');
  if ((readyFlag & 0x01) > 0) {
    //Serial.write("Ready Flag Checked", 18);
    inpToAction();
    readyFlag = readyFlag & !(0x01);  // Clear Serial Ready Flag
  }
  
  /*nonIntRead(incoming);
  if (incoming[0] != '\0') {
    Serial.println(incoming);
    Serial.println("calling inp");
    inpToAction();
  }*/

  
  if ((readyFlag >> 4) > 0) {
    int retVal = readyFlag >> 4;
    if (retVal == 2) {
      //Left Junction
      Serial.write("Left", 4);
    } else if (retVal == 3) {
      //Right Junction
      Serial.write("Right", 5);
    } else if (retVal == 4) {
      //T Junction
      Serial.write("T", 1);
    }

    readyFlag = readyFlag & !(0xF0);  //Clear Hall Effect Flag
  }
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
  //BT.write(ret);
  return ret;
  
}

void inpToAction(void) {
  Serial.println("Switch called");
  switch (incoming[0])
    {
      case ('F' | 'f'):
        dur = getDur(incoming);
        Motors.motorForward(dur);
        break;
  
      case ('B' | 'b'):
        dur = getDur(incoming);
        Motors.motorBack(dur);
        break;
  
      case ('L' | 'l'):;
        dur = getDur(incoming);
       // Motors.motorDir(1,dur);
        break;
  
      case ('R'| 'r'):
        dur = getDur(incoming);
        //Motors.motorDir(0,dur);
        break;

      case ('S' | 's'):
        //Serial.println("Stop called");
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
