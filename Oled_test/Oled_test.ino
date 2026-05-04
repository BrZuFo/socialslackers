#define CLK 2
#define DT 3
#define SW 4

int lastCLK;
int position = 0;

void setup() {
  Serial.begin(9600);

  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);

  lastCLK = digitalRead(CLK);

  Serial.println("Rotary encoder test");
}

void loop() {
  int currentCLK = digitalRead(CLK);

  if (currentCLK != lastCLK) {
    if (digitalRead(DT) != currentCLK) {
      position++;
      Serial.println("Right");
    } else {
      position--;
      Serial.println("Left");
    }

    Serial.print("Position: ");
    Serial.println(position);
  }

  lastCLK = currentCLK;

  if (digitalRead(SW) == LOW) {
    Serial.println("Button pressed");
    delay(250);
  }
}