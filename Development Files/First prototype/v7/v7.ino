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
    //Serial.println("Read Called");
    delay(100); 
    int x = 0;
    while(Serial.available()){
      inc[x++] = Serial.read();
      //Serial.print(c);
    }
    Serial.println(inc);
    //Serial.println("Read Completed");
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
    void ToggleDir() {digitalWrite(in1, (dir)); digitalWrite(in2,(!dir)); dir = (!dir);}
    void ToggleState() {digitalWrite(en, (!s)); s =!s;}
     
    private:
      bool dir = true;
      int in1, in2, en;
      bool s = 0;
};

class MotorBus {
  public:
    MotorBus(DCMotor motor1, DCMotor motor2): m1(motor1), m2(motor2) {}

    void highOff() {
      m1.lowOff();
      m2.lowOff();
      stateReset();
    }
  
    void motorForward(int Inpdur) {
      m1.ToggleState();
      m2.ToggleState();
      movEn = true;
      dur = Inpdur * 100;
      previousMillis = millis();
    }

    void motorInfForward(void) {
      m1.ToggleState();
      m2.ToggleState();
      movEn = true;
      inf = true;
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

    void motorDir(int left, int dur) {
      if (left) {
        m1.ToggleDir();
        leftRev = true;
      } else {
        m2.ToggleDir();
        rightRev = true;
      }
      motorForward(dur);
    }

    void clkCheck(unsigned long currentMillis) {
      if (movEn) {
        if (!inf) {
          if(currentMillis - previousMillis >= dur) {
            m1.ToggleState();
            m2.ToggleState();
            //previousMillis = currentMillis;
            stateReset();
          }
        }
      }
    }

    void d90(int left) {
      #define turnTime 1
      motorDir(left, turnTime);
    }

    void d180(void) {
      #define turnTime 2 //Double turn time above
      motorDir(1, turnTime);
    }

  private:
    DCMotor m1, m2;
    bool movEn, rev, inf, leftRev, rightRev = false;
    
    unsigned long previousMillis, dur;  

    void stateReset(void) {
      movEn = false;
      inf = false;
      if (rev) {
        toggleMotorDir();
        rev = false;
      }
      if (leftRev) {
        m1.ToggleDir();
        leftRev = false;
      }
      if (rightRev) {
        m2.ToggleDir();
        rightRev = false;
      }
    }
    
};

void setup() {
  
  Serial.begin(9600);

  //Interupt Setup - 1 per millisecond?
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

// Setup motors
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

}

void loop() {
/*---------------------------------------------------------------------------------------- 
`available commands:
F X Forward for x milliseconds
X Forward for infinity
B X Backward for x milliseconds
L X Left for x milliseconds
Q Left 90 Degrees
R X Right for x milliseconds
E Right 90 Degrees
R 180 Spin
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
  //Serial.println("Switch called");
  switch (incoming[0])
    {
      case ('F' | 'f'):
        dur = getDur(incoming);
        Motors.motorForward(dur);
        break;

      case ('X' | 'x'):
        Motors.motorInfForward();
        break;
  
      case ('B' | 'b'):
        dur = getDur(incoming);
        Motors.motorBack(dur);
        break;
  
      case ('L' | 'l'):
        dur = getDur(incoming);
       // Motors.motorDir(1,dur);
        break;

      case ('Q' | 'q'):
        Motors.d90(1);
        break;
  
      case ('R'| 'r'):
        dur = getDur(incoming);
        //Motors.motorDir(0,dur);
        break;

      case ('E' | 'e'):
        Motors.d90(0);
        break;

      case ('R' | 'r'):
        Motors.d180();
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
