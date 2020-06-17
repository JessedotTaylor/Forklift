 #include <SoftwareSerial.h>

SoftwareSerial BT(8,9);     //TX, RX

const int Y = 6;
const int X = 3;
const int Z = 2;

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
char incoming[80] = {'\0'};
bool waiting = false;

int uCount;
int flags = 0x00;
unsigned long int lastInp;
bool USEN = false;

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
        append("Right");
        while((divY - nodeCount) != targY & (divY - nodeCount + 1) != targY) {
          nodeCount++;
          append("Continue");
        }
        append("Left");
      } else {
        append("Left");
        while((divY + nodeCount) != targY & (divY + nodeCount + 1) != targY) {
          nodeCount++;
          append("Continue");
        }
       append("Right");
      }
      for(int x = 0; x <= targX; x++) {
        append("Continue / Forward");
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

void BTReady() {
    //Serial.println("Read Called");
    int x = 0;
    incoming[80] = {'\0'};
    while(BT.available()){
      incoming[x++] = BT.read();
    }
    BT.print(incoming);
    flags = 0x02;
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
  for (volatile int x = 0; x < USSCOUNT; x++) {
    trigger();
    retList[x] = ((pulseIn(USList[x], HIGH, 38000)*10) / 58);
    //Serial.println((pulseIn(USList[x], HIGH, 38000)));
    //Serial.println(USList[x]);
    //Serial.println(retList[x]);
    if ((retList[x] > 1) && (retList[x] < 30)) {
      flags = 0x01;
      //Serial.println("US fail");
      return;
    }
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
  

  //Interupt Setup - 1 per millisecond?
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);

}

SIGNAL(TIMER0_COMPA_vect) {
  if (uCount++ > 250) {
    checkUS();
    uCount = 0;
  }
  //Serial.println(uCount);

  checkBT();

  //Serial.println(serialCount);
  if ((serialCount++ >= 200) && waiting) {
    BTReady();
    serialCount = 0;
    waiting = false;
  } 
  if (serialCount > 200) {
    serialCount = 0;
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(flags + '0');
  //Serial.println(flags & 0x02);
  if ((flags & 0x02) > 0) {
    //Serial.println("Ready Flag Checked");
    Serial.write(incoming);
    flags = flags & !(0x02);  // Clear Serial Ready Flag
    //Serial.println(flags & 0x02);
  }

  if (((flags & 0x01) > 0) && USEN) { // Ultrasonic Check Fail
    Serial.write('s');
    BT.write('s');
    int blocked = 1;
    while (blocked == 1) {
      //Serial.println("blocked loop");
      flags = flags & !(0x01);
      //Serial.println(flags);
      checkUS();
      //Serial.println(flags);
      if ((flags & 0x01) == 0) {
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
      } else {
        delay(100);
      }
    }   
  }
}
