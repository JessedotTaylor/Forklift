//BT Setup
#include <SoftwareSerial.h>

SoftwareSerial BT(8,9);     //TX, RX

//Shelving Size Setup
const int Y = 6;
const int X = 3;
const int Z = 2;

//Define motor pins
#define PenA 11    //Code expecting left hand motor
#define Pin1 6
#define Pin2 5
#define PenB 10    //Code expecting right hand motor
#define Pin3 4
#define Pin4 3

// US setup
#define trig 13
#define US1 12
#define US2 11
#define US3 10
const int USSCOUNT = 3;

int USList[USSCOUNT] = {US1, US2, US3};
int retList[USSCOUNT] = {0,0,0};

int uCount;
int flags = 0x00;
unsigned long int lastInp;
bool USEN = true;


int checkLine = 0;
bool lineGood = true;
bool lineReFound = false;
int flCall = 0;
int lineCount = 0;

class pathList {
  public:
    void append(String item) {
      char cstr[item.length() + 1];
      strcpy(cstr,item.c_str());
      
      list[currentIndex++] = cstr;
      //currentIndex++;
    }
    
    char * pop(void) {
      return list[currentIndex--];
    }

    void wipe(void) {
      for (int x = 0; x <= 80; x++) {
        list[x] = {'\0'};
      }
      currentIndex = 0;
    }

    char * current(void) {
      return list[currentIndex];
    }

    void pathCalc(int targY, int targX, int targZ) {
      int divY = (Y / 2 - .5);
      int nodeCount = 1;
    
      if (targY < divY) {
        append(">");
        while((divY - nodeCount) != targY & (divY - nodeCount + 1) != targY) {
          nodeCount++;
          append("^");
        }
        append("<");
      } else {
        append("<");
        while((divY + nodeCount) != targY & (divY + nodeCount + 1) != targY) {
          nodeCount++;
          append("^");
        }
       append(">");
      }
      for(int x = 0; x <= targX; x++) {
        append("^");
      }
    
      if ((divY - nodeCount) == targY | (divY + nodeCount) == targY) {
        append("90 Right");
      } else {
        append("90 Left");
      }
    
      for(int x = 0; x <= targZ; x++) {
        append("Lift tines");
      }
  }

  private:
    char * list[80] = {'\0'};
    int currentIndex = 0;
};

void checkBT() {
  if (BT.available() && !waiting) {
    waiting = true;
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


void trigger() {
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
}

void checkUS() {
  Serial.println("checking US");
  for (volatile int x = 0; x < USSCOUNT; x++) {
    trigger();
    retList[x] = ((pulseIn(USList[x], HIGH, 38000)*10) / 58);
    //Serial.println((pulseIn(USList[x], HIGH, 38000)));
    //Serial.println(USList[x]);
    Serial.println(retList[x]);
    if ((retList[x] > 1) && (retList[x] < 20)) {
      flags = 0x01;
      //Serial.println("US fail");
      return;
    }
  }
  Serial.println("Finishing Check");
  
}


//IR Sensor Stuff
const int HALL_COUNT = 5;
const int GAIN_H = 5;
const int GAIN_L = 0;

int checkHallV2(void) {
  bool basic = true;
  int returns[HALL_COUNT] = {0,0,0,0,0};
  for (int x = 0; x < HALL_COUNT; x++) {
    returns[x] = map(analogRead(x), 0, 1023, 8, 0);
  }

  if (basic) {
    if (returns[1] > GAIN_H) {
      lineGood = false;
      return 5; //Veering Left
    } else if (returns[3] > GAIN_H) {
      lineGood = false;
      return 6; //Veering Right
    } else if (returns[2] > GAIN_H) {  // ADD ENCODING DATA ABOVE THIS CONDITIONAL
      lineReFound = true;
      return 1; // A-OK, Straight line
    } else {
      lineReFound = true;
      return 1;
    }
  } else {
    if (returns[0] > GAIN_H && returns[1] > GAIN_H && returns[3] > GAIN_H && returns[4] > GAIN_H) {
      return 4; //T Junction
    } else if (returns[3] > GAIN_H && returns[4] > GAIN_H) {
      return 3; // Right Junction
    } else if (returns[0] > GAIN_H && returns[1] > GAIN_H) {
      return 2; //Left junction
    } else if (returns[1] > GAIN_H) {
      lineGood = false;
      return 5; //Veering Left
    } else if (returns[3] > GAIN_H) {
      lineGood = false;
      return 6; //Veering Right
    } else if (returns[2] > GAIN_H) {  // ADD ENCODING DATA ABOVE THIS CONDITIONAL
      lineReFound = true;
      return 1; // A-OK, Straight line
    } else if (returns[0] > GAIN_H) {   // Far left Sensor Triggering
      return 11;
    } else if (returns[4] > GAIN_H) {   // Far right Sensor Triggering
      return 15;
    } else {
      lineReFound = true;
      return 1;
    }
  }
}

void findLine(void) {
  checkLine = 0;
  int ret = checkHallV2();
  if (ret > 1) { // Have line, regardless of location 
    switch (ret) 
    { 
      case (4): //If at T junction
//        Serial.print("s\n");
//        delay(150);
//        Serial.print("b 1");
        Serial.print("s\nb 1");
        flCall++;
        checkLine = 1;  //Flag loop to recursivly (ish) call function after 200ms
        break;
       
      case 5: case 14: case 15: // If veering left OR right sensors triggering when refinding line
//        Serial.print("s\n");
//        delay(150);
//        Serial.print("r 1");
        Serial.print("s\ny 1");
        flCall++;
        checkLine = 1;
        break;

      case 6: case 11: case 12: // If veering Right OR Left sensors triggering when refinding line
//        Serial.print("s\n");
//        delay(150);
//        Serial.print("l 1"); 
        Serial.print("s\nt 1");
        flCall++;
        checkLine = 1;
        break;
    /*
     case (0):
        lineGood = true;
        flCall = 0;
        if (flCall >= 2) { 
          Serial.print('c');
        }
        */

     case 1:
        //lineGood = true;
        flCall = 0;
        lineReFound = true;
        //if (flCall >= 2) { 
        // Serial.print('c');
        //}
        return;
     default:
       return;
     }
     return;
    
  } /*else { //Cannot Find line
    Serial.print('s');
    delay(150);
    Serial.print("t 5"); //Rotate through 180 while refinding line
    //Serial.print("s\0l 1");
    checkLine = 1;
    if (flCall++ > 10) {     
      Serial.print('c');
      BT.write("\nCannot re-find line\n");
      //Serial.print('s');
      while(1) {}
    }
  }*/
    
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
        motorForward(dur, 6);
      } else {
        motorForward(dur, 100);
      }
      
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

void inpToActionV2(char * p) {
  //Serial.println("Switch called");
  //Serial.println(p);
  //Motors.highOff();
  switch (p[0])
    {
      case ('F' | 'f'):
        dur = getDur(p);
        Motors.motorForward(dur, 100);
        break;

      case ('X' | 'x'):
        Motors.motorInfForward();
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
        Motors.highOff();
        break;
  
      case ('U' | 'u'):
        Serial.write("Unimplemented Command\n");
        break;
  
      case ('D' | 'd'):
        Serial.print("Unimplemented Command\n");
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

void setup() {
  // put your setup code here, to run once:
   pathList path;

    //Start software serial comms for BT control
  BT.begin(9600);
  Serial.begin(9600);

  pinMode(trig, OUTPUT);
  pinMode(US1, INPUT_PULLUP);
  pinMode(US2, INPUT_PULLUP);
  pinMode(US3, INPUT_PULLUP);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);

  

  //Interupt Setup - 1 per millisecond?
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

DCMotor motorA(Pin1, Pin2, PenA);
DCMotor motorB(Pin3, Pin4, PenB);

MotorBus Motors(motorA, motorB);

pathList path;

SIGNAL(TIMER0_COMPA_vect) {
  

  
  

  if ((serialCount++ >= 200) && waiting) {
    BTReady(incoming);
    serialCount = 0;
    waiting = false;
  } 
  
  if (serialCount > 200) {
    serialCount = 0;
  }

  
  

  
}

SIGNAL(TIMER0_COMPA_vect) {
  unsigned long currentMillis = millis();
  Motors.clkCheck(currentMillis);

  if (checkLine > 0) {
    checkLine++;
    if (checkLine > 200) {
      findLine();
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long TSL_Serial, TSL_US, TSL_Line;
  unsigned long currentMillis = millis();
  
  checkSerial();
  checkBT();

  if (((currentMillis - TSL_Serial) >= 200) && (waiting == 1)) {
    //Serial.println("over 200");
    TSL_Serial = currentMillis;
    serialReady(incoming);
    inpToActionV2(incoming);
  }

   if ((currentMillis - TSL_US) > 2000) {
    checkUS();
    TSL_US = currentMillis;
  }

  if ((currentMillis - TSL_Line) > 100) {
    lineFlag = checkHallV2();
    lineCount = currentMillis;
  }


  

}
