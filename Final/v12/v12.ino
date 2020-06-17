//Define motor pins
#define PenA 11    //Code expecting left hand motor
#define Pin1 4
#define Pin2 3
#define PenB 10    //Code expecting right hand motor
#define Pin3 6
#define Pin4 5

//Stepper Motor Defines

#define SPin1 7
#define SPin2 8
#define SPin3 12
#define SPin4 13
#define SenA A0
#define SenB A1


int getDur(char * p);


char c = ' ';
int data = 0;
int dur;


int serialCount = 0;
int readyFlag = 0;
const int INCOMING_BUFFER = 20;
char incoming[INCOMING_BUFFER] = {'\0'};
volatile int waiting = 0;

int seq1[4] = {1,0,0,1};
int seq2[4] = {1,1,0,0};
int seq3[4] = {0,1,1,0};
int seq4[4] = {0,0,1,1};
int * fullseq[4] = {seq1, seq2, seq3, seq4};

const unsigned int DELAY_BETWEEN_TICKS = 2;
unsigned long int STEPS;
int stepperCount = 0;

const int PREC_TIME = 11;


void checkSerial() {
  if (Serial.available() && !waiting) {
    //Serial.println("Checked and available");
    waiting = 1;
  }
}

void serialReady(char * inc) {
    //Serial.println("Read Called");
    int x = 0;
    while (x <= INCOMING_BUFFER) {
      inc[x] = '\0';
      x++;
    }
    x = 0;
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

    void blockingOn(int dur) {
      digitalWrite(en, HIGH);
      delay(dur);
      digitalWrite(en, LOW);
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

    void lowOff() {
      m1.lowOff();
      m2.lowOff();
      stateReset();
    }
  
    void motorForward(int Inpdur, int multi) {
      m1.ToggleState();
      m2.ToggleState();
      movEn = true;
      dur = Inpdur * multi;
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
      motorForward(dur, 100);
      //rev = false;
    }

    void toggleMotorDir(void) {
      m1.ToggleDir();
      m2.ToggleDir();
    }

    void motorDir(int left, int dur, bool prec) {
      if (left) {
        m1.ToggleDir();
        leftRev = true;
      } else {
        m2.ToggleDir();
        rightRev = true;
      }
      if (prec) {
        motorForward(dur, PREC_TIME);
      } else {
        motorForward(dur, 100);
      }
      
    }


    void clkCheck(unsigned long currentMillis) {
      //Serial.print("clk Check");
      if (movEn) {
        //Serial.print(" movEn");
        if (!inf) {
          //Serial.print(" !inf");
          if(currentMillis - previousMillis >= dur) {
            m1.ToggleState();
            m2.ToggleState();
            //previousMillis = currentMillis;
            stateReset();
          }
        }
      }
      //Serial.println();
    }

    void d90(int left) {
      #define turnTime 5
      motorDir(left, turnTime, false);
    }

    void d180(void) {
      #define turnTime 10 //Double turn time above
      motorDir(1, turnTime, false);
    }

  private:
    DCMotor m1, m2;
    bool movEn, rev, inf, leftRev, rightRev = false;
    
    unsigned long previousMillis, dur;  

    void stateReset(void) {
      noInterrupts();
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
      interrupts();
    }
    
};

class stepper {
  public:
    stepper(int ENA, int ENB,int IN1, int IN2, int IN3, int IN4): enA(ENA), enB(ENB), in1(IN1), in2(IN2), in3(IN3), in4(IN4) {
      pinMode(enA, OUTPUT);
      pinMode(enB, OUTPUT);
      pinMode(in1, OUTPUT);
      pinMode(in2, OUTPUT);
      pinMode(in3, OUTPUT);
      pinMode(in4, OUTPUT);
    }
  void writeBus(int p1, int p2, int p3, int p4) {
    digitalWrite(in1, p1);
    digitalWrite(in2, p2);
    digitalWrite(in3, p3);
    digitalWrite(in4, p4);
    
  }
  
  void lowOff() {
    digitalWrite(enA, LOW);
    digitalWrite(enB, LOW);
    digitalWrite(in1, LOW);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in1, LOW);
    s = 0;
  }

  void ToggleDir() {dir = !dir;}
  void ToggleState() {s = !s; digitalWrite(enA, s); digitalWrite(enB, s);}
  void stepperOn(int steps) {
    ToggleState();
    TStepCount = steps * 200;
  }

  /*void clkCheck(int wait) {
    if (s) {  //Stepper enabled
      if (count++ >= wait) { //Delay cleared
        writeBus(fullseq[tick][0],fullseq[tick][1],fullseq[tick][2],fullseq[tick][3]);
        count = 0;
        if (dir) { // Increment tick
          tick++;
          if (tick > 3) {
            tick = 0;
          }
        } else { // Decrement tick for reverse direction
          tick--;
          if (tick < 0) {
            tick = 1;
          }
        }
        stepCount++;
        if (stepCount >= TStepCount) {
          stepCount = 0;
          ToggleState();
          if (!(dir)) {
            ToggleDir();
          }
        }
      }
    }
  }*/

  void clkCheckV2(void) {
    if (s) {  //Stepper enabled
      writeBus(fullseq[tick][0],fullseq[tick][1],fullseq[tick][2],fullseq[tick][3]);
      if (dir) { // Increment tick
        tick++;
        if (tick > 3) {
          tick = 0;
        }
      } else { // Decrement tick for reverse direction
        tick--;
        if (tick < 0) {
          tick = 3;
        }
      }
      stepCount++;
      if (stepCount >= TStepCount) {
        stepCount = 0;
        ToggleState();
        if (!(dir)) {
          ToggleDir();
        }
      }
    }
  }

  
  private:
    bool dir = true;
    int enA, enB, in1, in2, in3, in4, dur;
    bool s = 0;
    int stepCount, tick, TStepCount = 0;
};


// Setup motors
DCMotor motorA(Pin1, Pin2, PenA);
DCMotor motorB(Pin3, Pin4, PenB);

MotorBus Motors(motorA, motorB);

stepper stepper1(SenA, SenB, SPin1, SPin2, SPin3, SPin4);


void setup() {
  
  Serial.begin(115200);
  
  //Interupt Setup - 1 per millisecond?
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
}


SIGNAL(TIMER0_COMPA_vect) {
  unsigned long currentMillis = millis();
  Motors.clkCheck(currentMillis);
  
  checkSerial();
  //Serial.println(waiting);
  if ((serialCount++ >= 5) && (waiting == 1)) {
    //Serial.println("over 200");
    serialCount = 0;
    waiting = 2;
  } 
  if (serialCount > 5) {
    serialCount = 0;
  }

  if (stepperCount++ >= DELAY_BETWEEN_TICKS) {
    stepper1.clkCheckV2();
    stepperCount = 0;
  }
  //stepper1.clkCheck(DELAY_BETWEEN_TICKS);

}

void loop() {
/*---------------------------------------------------------------------------------------- 
`available commands:
F X Forward for x milliseconds * 100
X Forward for infinity
I X Forward for x milliseconds * 10 (Precise Control)
B X Backward for x milliseconds * 100
L X Left for x milliseconds * 100
Q Left 90 Degrees
T X Left for x milliseconds * 10 (Precise Control)
R X Right for x milliseconds * 100
E Right 90 Degrees
Y X Right for x milliseconds * 10 (Precise Control)
C 180 Spin
U X Up for x turns (on stepper)
D X Down for x turns (on stepper)
M X Motor B for x Milliseconds (Blocking, Low Level)
N X Motor A for x Milliseconds (Blocking, Low Level)
EN Reenable US and Line Follower Sensors

*/
  if (waiting == 2) {
     serialReady(incoming);
  }
  //Serial.println(readyFlag + '0');
  if ((readyFlag & 0x01) > 0) {
    //Serial.write("Ready Flag Checked", 18);
    inpToActionV2(incoming);
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
  //Serial.println(dur);
  return ret;
  
}

void inpToActionV2(char * p) {
  //Serial.println("Switch called");
  //Serial.println(p);
  //Motors.lowOff();
  switch (p[0])
    {
      case ('F' | 'f'):
        dur = getDur(p);
        Motors.motorForward(dur, 100);
        break;

      case ('X' | 'x'):
        Motors.motorInfForward();
        break;
        
      case ('I' | 'i'):
        dur = getDur(p);
        Motors.motorForward(dur, PREC_TIME);
        break;
  
      case ('B' | 'b'):
        dur = getDur(p);
        Motors.motorBack(dur);
        break;
  
      case ('L' | 'l'):
        dur = getDur(p);
        Motors.motorDir(0,dur, false);
        break;

      case ('Q' | 'q'):
        Motors.d90(0);
        break;

      case ('T' | 't'):
        dur = getDur(p);
        Motors.motorDir(0,dur, true);
        break;
  
      case ('R'| 'r'):
        dur = getDur(p);
        Motors.motorDir(1,dur, false);
        break;

      case ('E' | 'e'):
        Motors.d90(1);
        break;

      case ('Y' | 'y'):
        dur = getDur(p);
        Motors.motorDir(1,dur, true);
        break;
      
      case ('C' | 'c'):
        Motors.d180();
        break;

      case ('S' | 's'):
        //Serial.println("Stop called");
        Motors.lowOff();
        stepper1.lowOff();
        break;
  
      case ('D' | 'd'):
        //Serial.println("Stepper DOWN");
        dur = getDur(p);
        stepper1.stepperOn(dur);
        break;
  
      case ('U' | 'u'):
        //Serial.println("Stepper UP");
        dur = getDur(p);
        stepper1.ToggleDir();
        stepper1.stepperOn(dur);
        
        break;

     case ('M' | 'm'):
        dur = getDur(p);
        motorB.blockingOn(dur);
        break;

     case ('N' | 'n'):
        dur = getDur(p);
        motorA.blockingOn(dur);
        break;
  
     case ('\n'):
        break;
        
     default:
        Serial.print("Unknown Command: ");
        Serial.println(p);
        break;
    }
    //Serial.println(p[1]);
    if (p[1] == '\\' | p[1] == '\n') {
      //Serial.println("Recursive call");
      int x = 2;
      if (p[1] == '\\') { int x = 3; }
      //int x = 3;
      char temp[INCOMING_BUFFER] = {'\0'};
      int z = 0;
      while (p[x] != '\0') {
        temp[z++] = p[x++];
      }
      //Serial.println(temp);
      inpToActionV2(temp);
    }
}
