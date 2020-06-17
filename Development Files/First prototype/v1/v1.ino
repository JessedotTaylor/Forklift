#include <SoftwareSerial.h>

//Define Bluetooth controls 
SoftwareSerial BT(8,9);     //TX, RX

//Define motor pins
#define enA 11    //Code expecting left hand motor
#define in1 5
#define in2 4
#define enB 10    //Code expecting right hand motor
#define in3 7
#define in4 6

bool checkOK();
void writeString(String stringData);
void motorForward(int dur);
void motorToggle(int l, int r);     // 1 if want to toggle left / right. Full reverse: motorToggle(1,1)
void motorBack(int dur);
void motorDir(int left, int dur);

int dir = 1;
char c = ' ';
int data = 0;
int dur = 1000;

void setup() {
  // Setup motors
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  // Set initial rotation direction
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  //Start software serial comms between BT control
  BT.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //BT.write("Loop Start");
  char inp[80] = {'\0'};
  int iter1,iter2 = 0;
  while (BT.available()) {
      c = BT.read();
      BT.write(c);
      BT.write("While");
      inp[iter1++] = c;
      data = 1;
    }
  /*int x = 0; 
  while (inp[x] != '\n') {
    BT.write(inp[x]);
    x++;
  }*/
/*---------------------------------------------------------------------------------------- 
`available commands:
F X Forward for x milliseconds
B X Backward for x milliseconds
L X Left for x milliseconds
R X Right for x milliseconds
U X Up for x turns (on stepper)
D X Down for x turns (on stepper)

*/
if (data) {
  //int dur = 0;
  int x = 1;
  while (inp[x] != '\0') {
    if (inp[x] != ' ') {
      dur = (inp[x] - '0') + (dur * 10);
      //BT.write(dur);
    }
    x++;
  }
  //BT.write(dur +'0');
  
  switch (inp[0])
  {
    case ('F' | 'f'):
      BT.write("Forward\n"); 
      //dur = getDur(inp);
      //BT.write("Forward %d\n" , dur);
      motorForward(dur);
      break;

    case ('B' | 'b'):
      BT.write("Backward\n");
      //dur = getDur(inp);
      motorBack(dur);
      break;

    case ('L' | 'l'):
      BT.write("Left\n");
      //dur = getDur(inp);
      motorDir(1,dur);
      break;

    case ('R'| 'r'):
      BT.write("Right\n");
      //dur = getDur(inp);
      motorDir(0,dur);
      break;

    case ('U' | 'u'):
      BT.write("Unimplemented Command\n");
      break;

    case ('D' | 'd'):
      BT.write("Unimplemented Command\n");
      break;

   case ('\n'):
      break;
      
   default:
      BT.write("Unknown Command\n");
      break;
  }
  BT.write("Loop reStart");
  data = 0;
}


}


/*int getDur(char * p) {
  int x = 1;
  int ret = 0;
  while (p[x] != '\n') {
    if (p[x] != ' ') {
      ret = p[x] + (ret * 10);
    }
    x++;
  }
  BT.write(ret);
  return ret;
  
}*/
bool checkOK() {
   // Checks for working BT module (Based on HM-10 Firmware)
//----------------------------------------------------------------------------------------  
  //Uncomment Serial println for and change Pen.read() to checkPrint() for debugging
  BT.write("AT");
  //Serial.println("AT sent");
  char ret[2] = {' '};
  int iter1 = 0;
  //delay(500);
  while (BT.available()) {
    c = BT.read();
    ret[iter1] = c;
    iter1++;
  } //Serial.print("\n");

  //Search for avaiable connections and store as list
//------------------------------------------------------------------------------------------
  //Based on success of above check
  if (ret[0] == 'O' and ret[1] == 'K') {
    return true;
  } else {
    return false;
}
}


void writeString(String stringData) { // Used to serially push out a String with Serial.write()
  int len = stringData.length();
  for (int i = 0; i < len; i++)
  {
    BT.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }

}// end writeString 

void motorToggle(int l, int r) {
  if (dir) {
    if(1) {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
    } 
    if (r) {
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
    }
    dir = 0;
  } else {
      if (l) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
      }
      if (r) {
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
      }
      dir = 1;
   }  
}


void motorForward(int dur) {
  BT.write("start");
  digitalWrite(enA, HIGH);
  digitalWrite(enB, HIGH);
  delay(dur);               //MAKE INTERUPT DRIVEN 
  digitalWrite(enA, LOW);
  digitalWrite(enB, LOW);
  BT.write("fin");
}

void motorBack(int dur) {
  motorToggle(1,1);
  motorForward(dur);
  motorToggle(1,1);
}

void motorDir(int left, int dur) {
  int l = left;
  int r = !left;
  
  motorToggle(l,r);
  motorForward(dur);
  motorToggle(l,r);
}

