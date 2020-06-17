 #include <SoftwareSerial.h>

SoftwareSerial BT(8,9);     //TX, RX

const int Y = 6;
const int X = 3;
const int Z = 2;

#define SenA 
#define SenB

#define trig 13
#define US1 12
#define US2 11
#define US3 10
const int USSCOUNT = 3;

#define LED_PIN 13

int USList[USSCOUNT] = {US1, US2, US3};
int retList[USSCOUNT] = {0,0,0};

char c;
int serialCount = 0;
//int readyFlag = 0;
int lineFlag = 0;
const int INCOMING_BUFFER = 80;
char incoming[INCOMING_BUFFER] = {'\0'};
bool waiting = false;

int uCount;
int flags = 0x00;
unsigned long int lastInp = 0;
bool USEN = false;
int USMaxDist = 20;
int timeOut = (((USMaxDist + 5) * 58) / 10) * 1000;


int checkLine = 0;
bool lineGood = true;
bool lineReFound = false;
int flCall = 0;
int lineCount = 0;
bool lineEN = true;
//bool lEN = true;    //returns[0]
//bool rEN = true;    //returns[2]

bool Send = true;
bool progState = true;

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


void BTReady(char * inc) {
    //Serial.println("Read Called");
    int x = 0;
    while (x <= INCOMING_BUFFER) {
      inc[x] = '\0';
      x++;
    }
    x = 0;
    while(BT.available()){
      inc[x++] = BT.read();
    }
    BT.print(inc);
    flags = 0x02;
    if (!progState) {
      lineFlag = checkHallV4();
      progState = true;
    }
   
    //Serial.println(flags + '0');
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
  //Serial.println("checking US");
  noInterrupts();
  for (volatile int x = 0; x < USSCOUNT; x++) {
    trigger();
    retList[x] = ((pulseIn(USList[x], HIGH, timeOut)*10) / 58);
    //Serial.println((pulseIn(USList[x], HIGH, 38000)));
    //Serial.println(USList[x]);
    //Serial.println(retList[x]);
    if ((retList[x] > 1) && (retList[x] < USMaxDist)) {
      flags = 0x01;
      //Serial.println("US fail");
      return;
    }
  }
  interrupts();
  //Serial.println("Finishing Check");
}

const int HALL_COUNT = 3;
const int GAIN_H = 2;
const int GAIN_L = 2;
/*
int checkHall(void) {
  int returns[HALL_COUNT] = {0,0,0,0,0};
  for (int x = 0; x < HALL_COUNT; x++) {
    returns[x] = map(analogRead(x), 0, 1023, 8, 0);
  }
  if (returns[0] > GAIN_H && returns[1] > GAIN_H && returns[2] > GAIN_H && returns[3] > GAIN_H && returns[4] > GAIN_H) {
    return 4; //T Junction
  } else if (returns[2] > GAIN_H && returns[3] > GAIN_H && returns[4] > GAIN_H) {
    return 3; // Right Junction
  } else if (returns[0] > GAIN_H && returns[1] > GAIN_H && returns[2] > GAIN_H) {
    return 2; //Left junction
  } else if (returns[1] > GAIN_H && returns[2] > GAIN_H) {
    lineGood = false;
    return 5; //Veering Left
  } else if (returns[2] > GAIN_H && returns[3] > GAIN_H) {
    lineGood = false;
    return 6; //Veering Right
  } else if (returns[2] == 0) {  // ADD ENCODING DATA ABOVE THIS CONDITIONAL
    return 1; // A-OK, Straight line
  } else if (returns[0] == 0) {   // Far left Sensor Triggering
    return 11;
  } else if (returns[4] == 0) {   // Far right Sensor Triggering
    return 15;
  } else if (returns[1] == 0) {   // Middle left Sensor Triggering
    return 12;
  } else if (returns[3] == 0) {   // Middle Right Sensor Triggering
    return 14;
  } else {
    lineGood = false;
    return 0;
  }
}
*/
/*
int checkHallV2(void) {
  bool basic = true;
  int returns[HALL_COUNT] = {0,0, 0};
  for (int x = 0; x < HALL_COUNT; x++) {
    returns[x] = map(analogRead(x), 0, 1023, 8, 0);
  }

  if (basic) {
    if (returns[0] < GAIN_H) {
      lineGood = false;
      return 5; //Veering Left
    } else if (returns[2] < GAIN_H) {
      lineGood = false;
      return 6; //Veering Right
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
*/
/*
int checkHallV3(void) {
  int returns[HALL_COUNT] = {0,0,0,0,0};
  int triggered[HALL_COUNT] = {0,0,0,0,0};
  const int middle = 1;     //Right 1 sensor
  for (int x = 0; x < HALL_COUNT; x++) {
    returns[x] = map(analogRead(x), 0, 1023, 8, 0);
    triggered[x] = returns[x] > GAIN_H;
  }

  
  int fLeft = triggered[0];
  int mLeft = triggered[1];
  int m = triggered[2];
  int mRight = triggered[3];
  int fRight = triggered[4];

      if (returns[0] > GAIN_H && returns[1] > GAIN_H && returns[2] > GAIN_H && returns[3] > GAIN_H && returns[4] > GAIN_H) {
      return 4; //T Junction
    } else if (returns[2] > GAIN_H && returns[3] > GAIN_H && returns[4] > GAIN_H) {
      return 3; // Right Junction
    } else if (returns[0] > GAIN_H && returns[1] > GAIN_H && returns[2] > GAIN_H) {
      return 2; //Left junction
    } else if (returns[1] > GAIN_H && returns[2] > GAIN_H) {
      lineGood = false;
      return 5; //Veering Left
    } else if (returns[2] > GAIN_H && returns[3] > GAIN_H) {
      lineGood = false;
      return 6; //Veering Right
    } else if (returns[2] == 0) {  // ADD ENCODING DATA ABOVE THIS CONDITIONAL
      return 1; // A-OK, Straight line
  

  

  
}*/

int checkHallV4(void) {
  //Serial.println("Check Called");
  int returns[HALL_COUNT] = {0,0,0};

  /*
  if (lEN) {returns[0] = digitalRead(A0); } else {returns[0] = 0;}
  returns[1] = digitalRead(A1);
  if (rEN) {returns[2] = digitalRead(A2); } else {returns[2] = 0;}
  */
  returns[0] = digitalRead(A0);
  returns[1] = digitalRead(A1);
  returns[2] = digitalRead(A2);
  /* 
  Serial.print('\n');
  Serial.print(lEN );
  Serial.print(',');
  Serial.print(returns[0]);
  Serial.print(' ');
  Serial.print(rEN );
  Serial.print(',');
  Serial.println(returns[2]);
  /*
  Serial.print(digitalRead(A0));
  Serial.print(' ');
  Serial.print(digitalRead(A1));
  Serial.print(' ');
  Serial.print(digitalRead(A2));
  */
  if (returns[0] && returns[2]) {
    lineGood = false;
    return 4;
  } else if (returns[2]) {
    lineGood = false;
    return 5;
  } else if (returns[0]) {
    lineGood = false;
    return 6;
  } else {
    //lineGood = true;
    return 1;
  }
}

/*
 * 1 = A-OK
 * 2 = Left Junction
 * 3 = Right Junction
 * 4 = T Junction
 * 5 = Veer left
 * 6 = Veer Right
 * 
 * 
 * 11 = Far Left
 * 12 = Middle Left
 * 14 = Middle Right
 * 15 = Far Right
 */


void findLine(void) {
  //Serial.print('s');
  checkLine = 0;
  int ret = checkHallV4();
  if (ret > 0) { // Have line, regardless of location 
    switch (ret) 
    { 
      /*case (4): //If at T junction
//        Serial.print("s\n");
//        delay(150);
//        Serial.print("b 1");
        Serial.print("s\nb 1");
        flCall++;
        checkLine = 1;  //Flag loop to recursivly (ish) call function after 200ms
        break;*/
       
      case 5: case 14: case 15: // If veering left OR right sensors triggering when refinding line
//        Serial.print("s\n");
//        delay(150);
//        Serial.print("r 1");
        Serial.print("s\ny 2\n i 1");
        flCall++;
        checkLine = 1;
        //lEN = false;
        break;

      case 6: case 11: case 12: // If veering Right OR Left sensors triggering when refinding line
//        Serial.print("s\n");
//        delay(150);
//        Serial.print("l 1"); 
        Serial.print("s\nt 2\ni 1");
        flCall++;
        checkLine = 1;
        //rEN = false;
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
        //BT.write("Case 1 call");
        lineReFound = true;
        //lEN = true;
        //rEN = true;
        //if (flCall >= 2) { 
        // Serial.print('c');
        //}
        return;
     default:
        //BT.write("Returning out");
       return;
    }
    
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
void setup() {
  // put your setup code here, to run once:
  pathList path;

    //Start software serial comms for BT control
  BT.begin(9600);
  Serial.begin(115200);

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

pathList path;

SIGNAL(TIMER0_COMPA_vect) {
  if (progState) {
    if (uCount++ > 2000 && USEN) {
      checkUS();
      uCount = 0;
    }
  
    if (uCount > 2000) {
      uCount = 0;
    }

    if ((lineCount++ > 25) && (checkLine == 0) ) {//&& lineEN) {
      lineFlag = checkHallV4();
      lineCount = 0;
    }
  }
  

  checkBT();
  
  if ((serialCount++ >= 5) && waiting) {
    BTReady(incoming);
    serialCount = 0;
    waiting = false;
  } 
  
  if (serialCount > 5) {
    serialCount = 0;
  }


  if (checkLine > 0) {
    checkLine++;
    if (checkLine > 200) {
      findLine();
      
    }
  }
}

int timeSinceLast(void) {
  unsigned long int cur = getDur(incoming);

  int durLeft = (cur - lastInp / 1000);
  BT.println();
  BT.write(durLeft);
  if (durLeft > 0) {
    Serial.write(incoming[0]);
    Serial.write(" ");
    Serial.write(durLeft);
    BT.write(incoming[0]);
    BT.write(" ");
    BT.write(durLeft);
    return 1;
  }
  lastInp = millis();
  return 0;
}

void loop() {
  
  if ((flags & 0x02) > 0) {
    //Serial.println("Ready Flag Checked");
    if (incoming[1] == 'N' | incoming[1] == 'n') {
      int x = 0;
      while (x <= INCOMING_BUFFER) {
        incoming[x] = '\0';
        x++;
      }
      progState = true;
    } else if (incoming[0] == 'U' | incoming[0] == 'u' | incoming[0] == 'D' | incoming[0] == 'd') {
      digitalWrite(SenA, HIGH);
      digitalWrite(SenB, HIGH);
      Serial.write(incoming);
      
    } else if (incoming[0] == 'S' | incoming[0] == 's') { 
      digitalWrite(SenA, LOW);
      digitalWrite(SenB, LOW);
      Serial.write(incoming);
      
    } else {
      Serial.write(incoming);
    }
    
    flags = flags & !(0x02);  // Clear Serial Ready Flag
    
    //Serial.println(flags & 0x02);
  }

  //Serial.println(incoming);
  
  if (((flags & 0x01) > 0) && USEN) { // Ultrasonic Check Fail
    Serial.write('s');
    BT.write('s');
    lineGood = false;
    int blocked = 1;
    while (blocked == 1) {
      //Serial.println("blocked loop");
      flags = flags & !(0x01);
      //Serial.println(flags);
      checkUS();
      //Serial.println(flags);
      if ((flags & 0x01) == 0) {
        if (timeSinceLast()) {
          blocked = 0;
        }
        /*
        //Serial.println("passed");
        blocked = 0;
        unsigned long int cur = getDur(incoming);
        //Serial.println(lastInp / 1000);
        //Serial.println(cur );
        
        int durLeft = (cur - lastInp / 1000);
        if (durLeft > 0) {
          //Serial.println(durLeft);
          Serial.write(incoming[0]);
          Serial.write(" ");
          Serial.print(durLeft);
        }
        lastInp = millis();
        */
      } else {
        delay(100);
      }
    }   
  }
  
/*
 * 1 = A-OK
 * 2 = Left Junction
 * 3 = Right Junction
 * 4 = T Junction
 * 5 = Veer left
 * 6 = Veer Right
 * 
 * 
 * 11 = Far Left
 * 12 = Middle Left
 * 14 = Middle Right
 * 15 = Far Right
 */

 if (lineFlag == 4) {
  if (progState) {
    if (Send) {Serial.println('s');}
    BT.write("T Junction, Stopping Program\n");
    int x = 0;
    while (x <= INCOMING_BUFFER) {
      incoming[x] = '\0';
      x++;
    }
    incoming[0] = 's';
    lineGood = true;
    lineFlag = checkHallV4();
    progState = false;
  }
 }
 /*
  if (lineFlag > 1) {
    bool Send = false;
    if (Send) {Serial.print('s');}
    if (lineFlag == 2) {
      //Left Junction
      BT.write("Left Junction");
      if (Send) {
        char * p = path.current();
        if (p[0]  == '<') {
          Serial.print('q'); //Turn left 90 degrees
          //delay(300);
        } 
        Serial.print('x');
        path.pop();
      }
    } else if (lineFlag == 3) {
      //Right Junction
      BT.write("Right Junction");
      if (Send) {
        char * p = path.current();
        if (p[0]  == '>') {
          Serial.print('e');  //Turn right 90 degrees
        } 
        Serial.print('x');    //Continue Forward
        path.pop();
      }
    } else if (lineFlag == 4) {
      //T Junction
      char * p = path.current();
      BT.write("T Junction");
      if (Send) {
        if (p[0]  == '<') {
          Serial.print('l');  //Turn left 90 degrees
          path.pop();
        } else if (p[0] == '>') {
          Serial.print('r');    //Turn Right 90 Degrees
          path.pop();
        } else if (p[0] == '^') {
          Serial.print('x');    //Continue Forward
          path.pop();
        } else {
          BT.println("\nT Junction, Confusion");
          BT.println(path.current());
        }
      } 
    }
    lineFlag = 0;  //Clear Hall Effect Flag
  }
*/
  if (!lineGood & lineReFound) {
    lineGood = true;
    lineReFound = false;
    //BT.write("Line Re-found. Sending last command\n");
    //timeSinceLast();
    Serial.print("s\n");
    Serial.print(incoming);
  }
  
  if (!lineGood & (checkLine == 0)) {
    //BT.print("Line not Good!\n");
    findLine();
  }

  
}
