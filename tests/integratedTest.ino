// imports OLED libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// initializes OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// initializes input device pins
#define CLK 2
#define DT 3
#define SW 4

// initializes variables
int lastCLK;
int position = 0;
int numOpt = 2;

// initializes lists for options and responses
String options[] = {"Yes", "No"};

// basic setup idk
void setup() {

  Serial.begin(9600);

  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);

  lastCLK = digitalRead(CLK);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.display();
}

// main loop for logic
void loop() {

  int currentCLK = digitalRead(CLK);

  // checks if cursor's current position has changed
  if (currentCLK != lastCLK) {

    if (digitalRead(DT) != currentCLK) {
      position++;
    } else {
      position--;
    }
  }

  // makes sure cursor is within bounds
  if (position < 0) {
    position = numOpt - 1;
  }

  if (position >= numOpt) {
    position = 0;
  }

  drawMenu();

  lastCLK = currentCLK;

  // if button clicked and main logic
  if (digitalRead(SW) == LOW) {

    display.clearDisplay();
    display.setCursor(0,0);

    if (options[position] == "Yes" || options[position] == "No") {

      display.println("It worked");
      display.display();
      delay(1000);

    } else {

      display.println("It didn't work");
      display.display();
      delay(1000);
    }
  }
}

// creates the images on OLED and sets up cursor
void drawMenu() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0,0);
  display.println("Choose option:");

  for (int i = 0; i < numOpt; i++) {

    if (i == position) {
      display.print("> ");
    } else {
      display.print("  ");
    }

    display.println(options[i]);
  }

  display.display();
}