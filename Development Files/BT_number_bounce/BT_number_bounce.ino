#include <SoftwareSerial.h>

//Define Bluetooth controls 
SoftwareSerial BT(8,9);     //TX, RX

char c = '\0';

void setup() {
  // put your setup code here, to run once:
  BT.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
   while (BT.available()) {
      c = BT.read();
      BT.write(c);
   }
}
