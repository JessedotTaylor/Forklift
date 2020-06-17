#define Y = 6
#define X = 3
#define Z = 2

class pathList
{
  public:
    void append(String item) {
      list[currentIndex++] = item;
      //currentIndex++;
    }
    
    String pop(void) {
      return list[currentIndex--];
    }

    void wipe(void) {
      list = [];
      currentIndex = 0;
    }

    String current(void) {
      return list[currentIndex];
    }

  private:
    list = [];
    currentIndex = 0;
}

void pathCalc(int targY, targX, targZ) {
  divY = int(Y / 2 - .5);
  nodeCount = 1;

  if (targY < divY) {
    path.append("Right");
    while((divY - nodeCount) != targY & (divY - nodeCount + 1) != targY) {
      nodeCount++;
      path.append("Continue");
    }
    path.append("Left")
  } else {
    path.append("Left")
    while((divY + nodeCount) != targY & (divY + nodeCount + 1) != targY) {
      nodeCount++;
      path.append("Continue");
    }
    path.append("Right");
  }
  for(int x = 0; x <= targX; x++) {
    path.append("Continue / Forward");
  }

  if ((divY - nodeCount) == targY | (divY + nodeCount) == targY) {
    path.append("90 Right");
  } else {
    path.append("90 Left");
  }

  for(int x = 0; x <= targZ; x++) {
    path.append("Lift tines");
  }
}


void setup() {
  // put your setup code here, to run once:
  pathList path;

  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

}


