// ============================================================
// SOCIAL SIM — Arduino OLED Edition
// Hardware: 128x64 SSD1306 OLED, rotary encoder (CLK/DT/SW)
//
// Flow:
//   Choose setting → Choose person → Browse options one at a time
//   (encoder cycles, text marquee-scrolls if too wide) →
//   Press to confirm → Play again?
// ============================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>

// ---------- OLED ----------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- ENCODER PINS ----------
#define CLK 2
#define DT  3
#define SW  4

// ---------- MARQUEE ----------
#define MARQUEE_Y        38   // pixel row the scrolling text sits on
#define MARQUEE_SPEED_MS  30  // ms per pixel shift (lower = faster)
#define MARQUEE_GAP       10  // blank pixels between loop-around

// ============================================================
// ENUM INDICES
// ============================================================
enum SettingIdx : uint8_t { S_WORKPLACE=0, S_SCHOOL, S_PUBLIC, S_BATHROOM, NUM_SETTINGS };
enum PersonIdx  : uint8_t { P_BOSS=0, P_FRIEND, P_COWORKER, P_STRANGER,   NUM_PEOPLE   };
enum VibeIdx    : uint8_t { V_POLITE=0, V_TENSE, V_SARCASTIC, V_RUDE, V_AWKWARD, NUM_VIBES };

// ============================================================
// LABEL STRINGS — flash
// ============================================================
const char s0[] PROGMEM = "workplace";
const char s1[] PROGMEM = "school";
const char s2[] PROGMEM = "public";
const char s3[] PROGMEM = "bathroom";
const char* const settings[] PROGMEM = { s0, s1, s2, s3 };

const char p0[] PROGMEM = "boss";
const char p1[] PROGMEM = "friend";
const char p2[] PROGMEM = "co-worker";
const char p3[] PROGMEM = "stranger";
const char* const people[] PROGMEM = { p0, p1, p2, p3 };

const char v0[] PROGMEM = "polite";
const char v1[] PROGMEM = "tense";
const char v2[] PROGMEM = "sarcastic";
const char v3[] PROGMEM = "rude";
const char v4[] PROGMEM = "awkward";
const char* const vibes[] PROGMEM = { v0, v1, v2, v3, v4 };

void pgmLabel(const char* const* table, uint8_t idx, char* buf, uint8_t bufLen) {
  strcpy_P(buf, (char*)pgm_read_ptr(&table[idx]));
}

// ============================================================
// RESPONSE TABLE
// ============================================================
struct ResponseEntry {
  uint8_t setting;
  uint8_t person;
  uint8_t vibe;
  const char* line;   // PROGMEM pointer
};

const char r000[] PROGMEM = "Report's done, sir.";
const char r001[] PROGMEM = "Need anything else?";
const char r010[] PROGMEM = "I'll fix it by 5.";
const char r020[] PROGMEM = "Sure, I'll do it again.";
const char r030[] PROGMEM = "Do it yourself.";
const char r040[] PROGMEM = "You emailed me?";
const char r100[] PROGMEM = "Got a sec to review?";
const char r110[] PROGMEM = "Stay in your lane.";
const char r120[] PROGMEM = "Oh, you noticed too?";
const char r130[] PROGMEM = "Not my problem.";
const char r140[] PROGMEM = "Haha... anyway...";
const char r200[] PROGMEM = "Lunch later?";
const char r210[] PROGMEM = "We need to talk.";
const char r220[] PROGMEM = "Great meeting, huh?";
const char r230[] PROGMEM = "You're late. Again.";
const char r240[] PROGMEM = "How's the... stuff?";
const char r300[] PROGMEM = "Can I help you?";
const char r310[] PROGMEM = "Sign in at the desk.";
const char r320[] PROGMEM = "Visiting? How brave.";
const char r330[] PROGMEM = "Employees only.";
const char r340[] PROGMEM = "Are you... lost?";
const char r400[] PROGMEM = "Assignment submitted!";
const char r410[] PROGMEM = "But I did study...";
const char r420[] PROGMEM = "Fascinating lecture.";
const char r430[] PROGMEM = "This class is pointless.";
const char r440[] PROGMEM = "Wrong classroom, sorry.";
const char r500[] PROGMEM = "Can I copy the notes?";
const char r510[] PROGMEM = "You told everyone?!";
const char r520[] PROGMEM = "Oh sure, YOU get it.";
const char r530[] PROGMEM = "Save me a seat.";
const char r540[] PROGMEM = "We still hang out?";
const char r600[] PROGMEM = "Study together?";
const char r610[] PROGMEM = "You skipped your part.";
const char r620[] PROGMEM = "Oh you read it. Nice.";
const char r630[] PROGMEM = "This group is useless.";
const char r640[] PROGMEM = "So... who presents?";
const char r700[] PROGMEM = "Is this seat taken?";
const char r710[] PROGMEM = "That's my usual seat.";
const char r720[] PROGMEM = "Love your highlighter.";
const char r730[] PROGMEM = "Stop kicking my chair.";
const char r740[] PROGMEM = "Heh... same major?";
const char r800[] PROGMEM = "Fancy seeing you here!";
const char r810[] PROGMEM = "About that email...";
const char r820[] PROGMEM = "You shop here? Wow.";
const char r830[] PROGMEM = "I'm off the clock.";
const char r840[] PROGMEM = "Pretend you saw nothing.";
const char r900[] PROGMEM = "Hey! What are you up to?";
const char r910[] PROGMEM = "We need to talk. Now.";
const char r920[] PROGMEM = "You DO leave the house.";
const char r930[] PROGMEM = "You owe me money still.";
const char r940[] PROGMEM = "Did you get a haircut?";
const char ra00[] PROGMEM = "Weird running into you!";
const char ra10[] PROGMEM = "I saw your Slack msg.";
const char ra20[] PROGMEM = "Following me?";
const char ra30[] PROGMEM = "Don't tell HR.";
const char ra40[] PROGMEM = "So we just... wave?";
const char rb00[] PROGMEM = "Nice weather today?";
const char rb10[] PROGMEM = "That was my spot.";
const char rb20[] PROGMEM = "Eye contact now?";
const char rb30[] PROGMEM = "Watch where you're going!";
const char rb40[] PROGMEM = "Thought you were someone.";
const char rc00[] PROGMEM = "Pretend we're not here?";
const char rc10[] PROGMEM = "Report can wait, right?";
const char rc20[] PROGMEM = "Review in here?";
const char rc30[] PROGMEM = "No meetings. Ever.";
const char rc40[] PROGMEM = "...";
const char rd00[] PROGMEM = "Gossip break?";
const char rd10[] PROGMEM = "Why did you do that?";
const char rd20[] PROGMEM = "Hiding too, huh?";
const char rd30[] PROGMEM = "You took forever.";
const char rd40[] PROGMEM = "I'll wait outside.";
const char re00[] PROGMEM = "Busy day, right?";
const char re10[] PROGMEM = "I heard what you said.";
const char re20[] PROGMEM = "Great chat spot.";
const char re30[] PROGMEM = "Don't talk to me here.";
const char re40[] PROGMEM = "We'll just not talk.";
const char rf00[] PROGMEM = "Dryer's broken, FYI.";
const char rf10[] PROGMEM = "There's a line, y'know.";
const char rf20[] PROGMEM = "Love what they've done.";
const char rf30[] PROGMEM = "Stop whistling.";
const char rf40[] PROGMEM = "Uh... nice shoes.";

const ResponseEntry PROGMEM responseTable[] = {
  {S_WORKPLACE, P_BOSS,     V_POLITE,    r000},
  {S_WORKPLACE, P_BOSS,     V_POLITE,    r001},
  {S_WORKPLACE, P_BOSS,     V_TENSE,     r010},
  {S_WORKPLACE, P_BOSS,     V_SARCASTIC, r020},
  {S_WORKPLACE, P_BOSS,     V_RUDE,      r030},
  {S_WORKPLACE, P_BOSS,     V_AWKWARD,   r040},
  {S_WORKPLACE, P_COWORKER, V_POLITE,    r100},
  {S_WORKPLACE, P_COWORKER, V_TENSE,     r110},
  {S_WORKPLACE, P_COWORKER, V_SARCASTIC, r120},
  {S_WORKPLACE, P_COWORKER, V_RUDE,      r130},
  {S_WORKPLACE, P_COWORKER, V_AWKWARD,   r140},
  {S_WORKPLACE, P_FRIEND,   V_POLITE,    r200},
  {S_WORKPLACE, P_FRIEND,   V_TENSE,     r210},
  {S_WORKPLACE, P_FRIEND,   V_SARCASTIC, r220},
  {S_WORKPLACE, P_FRIEND,   V_RUDE,      r230},
  {S_WORKPLACE, P_FRIEND,   V_AWKWARD,   r240},
  {S_WORKPLACE, P_STRANGER, V_POLITE,    r300},
  {S_WORKPLACE, P_STRANGER, V_TENSE,     r310},
  {S_WORKPLACE, P_STRANGER, V_SARCASTIC, r320},
  {S_WORKPLACE, P_STRANGER, V_RUDE,      r330},
  {S_WORKPLACE, P_STRANGER, V_AWKWARD,   r340},
  {S_SCHOOL,    P_BOSS,     V_POLITE,    r400},
  {S_SCHOOL,    P_BOSS,     V_TENSE,     r410},
  {S_SCHOOL,    P_BOSS,     V_SARCASTIC, r420},
  {S_SCHOOL,    P_BOSS,     V_RUDE,      r430},
  {S_SCHOOL,    P_BOSS,     V_AWKWARD,   r440},
  {S_SCHOOL,    P_FRIEND,   V_POLITE,    r500},
  {S_SCHOOL,    P_FRIEND,   V_TENSE,     r510},
  {S_SCHOOL,    P_FRIEND,   V_SARCASTIC, r520},
  {S_SCHOOL,    P_FRIEND,   V_RUDE,      r530},
  {S_SCHOOL,    P_FRIEND,   V_AWKWARD,   r540},
  {S_SCHOOL,    P_COWORKER, V_POLITE,    r600},
  {S_SCHOOL,    P_COWORKER, V_TENSE,     r610},
  {S_SCHOOL,    P_COWORKER, V_SARCASTIC, r620},
  {S_SCHOOL,    P_COWORKER, V_RUDE,      r630},
  {S_SCHOOL,    P_COWORKER, V_AWKWARD,   r640},
  {S_SCHOOL,    P_STRANGER, V_POLITE,    r700},
  {S_SCHOOL,    P_STRANGER, V_TENSE,     r710},
  {S_SCHOOL,    P_STRANGER, V_SARCASTIC, r720},
  {S_SCHOOL,    P_STRANGER, V_RUDE,      r730},
  {S_SCHOOL,    P_STRANGER, V_AWKWARD,   r740},
  {S_PUBLIC,    P_BOSS,     V_POLITE,    r800},
  {S_PUBLIC,    P_BOSS,     V_TENSE,     r810},
  {S_PUBLIC,    P_BOSS,     V_SARCASTIC, r820},
  {S_PUBLIC,    P_BOSS,     V_RUDE,      r830},
  {S_PUBLIC,    P_BOSS,     V_AWKWARD,   r840},
  {S_PUBLIC,    P_FRIEND,   V_POLITE,    r900},
  {S_PUBLIC,    P_FRIEND,   V_TENSE,     r910},
  {S_PUBLIC,    P_FRIEND,   V_SARCASTIC, r920},
  {S_PUBLIC,    P_FRIEND,   V_RUDE,      r930},
  {S_PUBLIC,    P_FRIEND,   V_AWKWARD,   r940},
  {S_PUBLIC,    P_COWORKER, V_POLITE,    ra00},
  {S_PUBLIC,    P_COWORKER, V_TENSE,     ra10},
  {S_PUBLIC,    P_COWORKER, V_SARCASTIC, ra20},
  {S_PUBLIC,    P_COWORKER, V_RUDE,      ra30},
  {S_PUBLIC,    P_COWORKER, V_AWKWARD,   ra40},
  {S_PUBLIC,    P_STRANGER, V_POLITE,    rb00},
  {S_PUBLIC,    P_STRANGER, V_TENSE,     rb10},
  {S_PUBLIC,    P_STRANGER, V_SARCASTIC, rb20},
  {S_PUBLIC,    P_STRANGER, V_RUDE,      rb30},
  {S_PUBLIC,    P_STRANGER, V_AWKWARD,   rb40},
  {S_BATHROOM,  P_BOSS,     V_POLITE,    rc00},
  {S_BATHROOM,  P_BOSS,     V_TENSE,     rc10},
  {S_BATHROOM,  P_BOSS,     V_SARCASTIC, rc20},
  {S_BATHROOM,  P_BOSS,     V_RUDE,      rc30},
  {S_BATHROOM,  P_BOSS,     V_AWKWARD,   rc40},
  {S_BATHROOM,  P_FRIEND,   V_POLITE,    rd00},
  {S_BATHROOM,  P_FRIEND,   V_TENSE,     rd10},
  {S_BATHROOM,  P_FRIEND,   V_SARCASTIC, rd20},
  {S_BATHROOM,  P_FRIEND,   V_RUDE,      rd30},
  {S_BATHROOM,  P_FRIEND,   V_AWKWARD,   rd40},
  {S_BATHROOM,  P_COWORKER, V_POLITE,    re00},
  {S_BATHROOM,  P_COWORKER, V_TENSE,     re10},
  {S_BATHROOM,  P_COWORKER, V_SARCASTIC, re20},
  {S_BATHROOM,  P_COWORKER, V_RUDE,      re30},
  {S_BATHROOM,  P_COWORKER, V_AWKWARD,   re40},
  {S_BATHROOM,  P_STRANGER, V_POLITE,    rf00},
  {S_BATHROOM,  P_STRANGER, V_TENSE,     rf10},
  {S_BATHROOM,  P_STRANGER, V_SARCASTIC, rf20},
  {S_BATHROOM,  P_STRANGER, V_RUDE,      rf30},
  {S_BATHROOM,  P_STRANGER, V_AWKWARD,   rf40},
};
const uint8_t NUM_RESPONSES = sizeof(responseTable) / sizeof(ResponseEntry);

// ============================================================
// OPTION STRUCT  (no stats — removed with rating system)
// ============================================================
struct Option {
  uint8_t    vibeIdx;
  const char* text;    // PROGMEM pointer
};

// ============================================================
// ENCODER / BUTTON STATE
// ============================================================
int           lastCLK;
int           encPosition  = 0;
int           encReadCount = 0;
bool          btnPressed   = false;
unsigned long btnDebounce  = 0;

// ============================================================
// GAME STATE
// ============================================================
enum GameState : uint8_t {
  STATE_CHOOSE_SETTING,
  STATE_CHOOSE_PERSON,
  STATE_SHOW_SCENARIO,
  STATE_CHOOSE_OPTION,   // browse one option at a time + marquee
  STATE_SHOW_CHOICE,     // confirm chosen option
  STATE_PLAY_AGAIN
};

GameState gameState     = STATE_CHOOSE_SETTING;
uint8_t   chosenSetting = 0;
uint8_t   chosenPerson  = 0;
uint8_t   chosenOption  = 0;
uint8_t   chosenVibes[3];
Option    currentOptions[3];

// ============================================================
// MARQUEE STATE
// ============================================================
// textBuf holds the RAM copy of whichever response is on screen.
// marqueeX is the current left-edge pixel of the scrolling string.
// marqueeW is the total pixel width of the string (6px per char at textSize 1).
// If marqueeW <= SCREEN_WIDTH the text is static (centred).

char          textBuf[48];          // RAM copy of current response text
int16_t       marqueeX      = 0;    // current scroll offset (pixels)
int16_t       marqueeW      = 0;    // total text pixel width
unsigned long marqueeLastMs = 0;    // last tick timestamp
uint8_t       lastShownOpt  = 255;  // detect option change → reset marquee

// Load option idx into textBuf and reset marquee
void loadMarquee(uint8_t optIdx) {
  if (currentOptions[optIdx].text)
    strncpy_P(textBuf, currentOptions[optIdx].text, sizeof(textBuf) - 1);
  else
    strcpy(textBuf, "???");
  textBuf[sizeof(textBuf) - 1] = '\0';

  // At textSize 1, each character is 6 pixels wide
  marqueeW      = strlen(textBuf) * 6;
  marqueeX      = 0;
  marqueeLastMs = millis();
  lastShownOpt  = optIdx;
}

// Advance marquee by one pixel if enough time has passed.
// Returns true if a redraw is needed.
bool tickMarquee() {
  if (marqueeW <= SCREEN_WIDTH) return false;   // fits — no scrolling needed
  unsigned long now = millis();
  if (now - marqueeLastMs >= MARQUEE_SPEED_MS) {
    marqueeLastMs = now;
    marqueeX--;
    // Wrap: once the text has fully scrolled off, restart after a gap
    if (marqueeX < -(marqueeW + MARQUEE_GAP))
      marqueeX = 0;
    return true;
  }
  return false;
}

// ============================================================
// RANDOM HELPERS
// ============================================================
uint8_t randRange(uint8_t lo, uint8_t hi) {
  return lo + (rand() % (hi - lo + 1));
}

void pickVibes() {
  uint8_t pool[NUM_VIBES];
  for (uint8_t i = 0; i < NUM_VIBES; i++) pool[i] = i;
  for (uint8_t i = 0; i < 3; i++) {
    uint8_t j = randRange(i, NUM_VIBES - 1);
    uint8_t t = pool[i]; pool[i] = pool[j]; pool[j] = t;
    chosenVibes[i] = pool[i];
  }
}

const char* lookupResponse(uint8_t si, uint8_t pi, uint8_t vi) {
  const char* matches[4];
  uint8_t count = 0;
  for (uint8_t i = 0; i < NUM_RESPONSES && count < 4; i++) {
    ResponseEntry e;
    memcpy_P(&e, &responseTable[i], sizeof(ResponseEntry));
    if (e.setting == si && e.person == pi && e.vibe == vi)
      matches[count++] = e.line;
  }
  if (!count) return nullptr;
  return matches[randRange(0, count - 1)];
}

void buildOptions() {
  for (uint8_t i = 0; i < 3; i++) {
    currentOptions[i].vibeIdx = chosenVibes[i];
    currentOptions[i].text    = lookupResponse(chosenSetting, chosenPerson, chosenVibes[i]);
  }
}

// ============================================================
// DISPLAY HELPERS
// ============================================================
void drawTitleBar(const __FlashStringHelper* title) {
  display.fillRect(0, 0, SCREEN_WIDTH, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 1);
  display.print(title);
  display.setTextColor(SSD1306_WHITE);
}

// Scrollable list for PROGMEM string tables (settings / people)
void drawMenuPgm(const char* title, const char* const* pgmTable, uint8_t numOpts, uint8_t sel) {
  display.clearDisplay();
  display.setTextSize(1);
  display.fillRect(0, 0, SCREEN_WIDTH, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 1); display.print(title);
  display.setTextColor(SSD1306_WHITE);

  uint8_t start = (sel >= 4) ? sel - 3 : 0;
  char lbl[12];
  for (uint8_t i = 0; i < 4 && (start + i) < numOpts; i++) {
    uint8_t idx = start + i;
    int y = 17 + i * 12;
    pgmLabel(pgmTable, idx, lbl, sizeof(lbl));
    if (idx == sel) {
      display.fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(2, y); display.print(F("> "));
    } else {
      display.setCursor(2, y); display.print(F("  "));
    }
    display.print(lbl);
    display.setTextColor(SSD1306_WHITE);
  }
  display.display();
}

// Scrollable list for plain RAM string arrays (play-again)
void drawMenuRam(const char* title, const char** ramTable, uint8_t numOpts, uint8_t sel) {
  display.clearDisplay();
  display.setTextSize(1);
  display.fillRect(0, 0, SCREEN_WIDTH, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 1); display.print(title);
  display.setTextColor(SSD1306_WHITE);

  uint8_t start = (sel >= 4) ? sel - 3 : 0;
  for (uint8_t i = 0; i < 4 && (start + i) < numOpts; i++) {
    uint8_t idx = start + i;
    int y = 17 + i * 12;
    if (idx == sel) {
      display.fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(2, y); display.print(F("> "));
    } else {
      display.setCursor(2, y); display.print(F("  "));
    }
    display.print(ramTable[idx]);
    display.setTextColor(SSD1306_WHITE);
  }
  display.display();
}

// Draw the single-option card with live marquee text.
// Layout (128×64):
//   Row  0–9  : title bar  "Option X / 3"
//   Row 11–19 : vibe tag   "[sarcastic]"
//   Row 25–34 : separator line
//   Row 36    : marquee text (scrolling or static)
//   Row 54–63 : hint       "< turn > | press"
void drawOptionCard(uint8_t optIdx) {
  char lbl[16];

  display.clearDisplay();
  display.setTextSize(1);

  // --- Title bar ---
  display.fillRect(0, 0, SCREEN_WIDTH, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 1);
  display.print(F("Option "));
  display.print(optIdx + 1);
  display.print(F(" / 3"));
  display.setTextColor(SSD1306_WHITE);

  // --- Vibe tag ---
  pgmLabel(vibes, currentOptions[optIdx].vibeIdx, lbl, sizeof(lbl));
  display.setCursor(0, 18);
  display.print('[');
  display.print(lbl);
  display.print(']');

  // --- Separator ---
  display.drawFastHLine(0, 30, SCREEN_WIDTH, SSD1306_WHITE);

  // --- Marquee / static text ---
  // Clip drawing to the text row (simple: just draw at the scrolled X)
  if (marqueeW <= SCREEN_WIDTH) {
    // Static: centre it
    int16_t cx = (SCREEN_WIDTH - marqueeW) / 2;
    display.setCursor(cx, MARQUEE_Y);
    display.print(textBuf);
  } else {
    // Scrolling: draw twice so the seamless loop wraps correctly
    display.setCursor(marqueeX, MARQUEE_Y);
    display.print(textBuf);

    display.setCursor(marqueeX + marqueeW + MARQUEE_GAP, MARQUEE_Y);
    display.print(textBuf);
  }

  // --- Bottom hint ---
  display.drawFastHLine(0, 52, SCREEN_WIDTH, SSD1306_WHITE);
  display.setCursor(4, 55);
  display.print(F("< turn >  | press OK"));

  display.display();
}

// ============================================================
// ENCODER / BUTTON
// ============================================================
void updateEncoder(uint8_t maxVal) {
  int cur = digitalRead(CLK);
  if (cur != lastCLK) {
    if (encReadCount % 2 == 0) {
      if (digitalRead(DT) != cur) encPosition++;
      else                        encPosition--;
    }
    encReadCount++;
  }
  lastCLK = cur;
  if (encPosition < 0)       encPosition = maxVal - 1;
  if (encPosition >= maxVal) encPosition = 0;
}

bool buttonClicked() {
  if (digitalRead(SW) == LOW && !btnPressed && millis() - btnDebounce > 300) {
    btnPressed  = true;
    btnDebounce = millis();
    return true;
  }
  if (digitalRead(SW) == HIGH) btnPressed = false;
  return false;
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(9600);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT,  INPUT_PULLUP);
  pinMode(SW,  INPUT_PULLUP);
  lastCLK = digitalRead(CLK);

  randomSeed(analogRead(A0));
  srand(analogRead(A1));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    for (;;);
  }
  display.setTextWrap(false);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println(F("SOCIAL SIM"));
  display.setCursor(4, 34);
  display.println(F("Press to start"));
  display.display();
}

// ============================================================
// MAIN LOOP
// ============================================================
void loop() {
  char lbl[16];

  switch (gameState) {

    // --------------------------------------------------------
    case STATE_CHOOSE_SETTING:
      updateEncoder(NUM_SETTINGS);
      drawMenuPgm("Setting?", settings, NUM_SETTINGS, encPosition);
      if (buttonClicked()) {
        chosenSetting = encPosition;
        encPosition   = 0;
        gameState     = STATE_CHOOSE_PERSON;
      }
      break;

    // --------------------------------------------------------
    case STATE_CHOOSE_PERSON:
      updateEncoder(NUM_PEOPLE);
      drawMenuPgm("Who?", people, NUM_PEOPLE, encPosition);
      if (buttonClicked()) {
        chosenPerson = encPosition;
        encPosition  = 0;
        gameState    = STATE_SHOW_SCENARIO;
      }
      break;

    // --------------------------------------------------------
    case STATE_SHOW_SCENARIO:
      pickVibes();
      buildOptions();
      display.clearDisplay();
      display.setTextSize(1);
      drawTitleBar(F("SCENARIO"));
      display.setCursor(0, 18); display.print(F("You see a "));
      pgmLabel(people, chosenPerson, lbl, sizeof(lbl));
      display.print(lbl);
      display.setCursor(0, 30); display.print(F("@ "));
      pgmLabel(settings, chosenSetting, lbl, sizeof(lbl));
      display.print(lbl);
      display.setCursor(0, 48); display.print(F("Press to respond"));
      display.display();
      if (buttonClicked()) {
        encPosition  = 0;
        lastShownOpt = 255;   // force marquee init on first option
        gameState    = STATE_CHOOSE_OPTION;
      }
      break;

    // --------------------------------------------------------
    case STATE_CHOOSE_OPTION: {
      updateEncoder(3);

      uint8_t opt = (uint8_t)encPosition;

      // If the user spun to a new option, reload the marquee
      if (opt != lastShownOpt) loadMarquee(opt);

      // Advance marquee timer; redraw whenever it ticks or first load
      bool needDraw = tickMarquee() || (opt != lastShownOpt);
      // Note: lastShownOpt is set inside loadMarquee, so the second
      // condition is always false after the first pass — tickMarquee
      // drives redraws thereafter.
      drawOptionCard(opt);   // always redraw (cheap on OLED buffer)

      if (buttonClicked()) {
        chosenOption = opt;
        encPosition  = 0;
        gameState    = STATE_SHOW_CHOICE;
      }
      break;
    }

    // --------------------------------------------------------
    case STATE_SHOW_CHOICE: {
      // Show the confirmed pick, then go straight to play-again.
      display.clearDisplay();
      display.setTextSize(1);
      drawTitleBar(F("You say:"));

      pgmLabel(vibes, currentOptions[chosenOption].vibeIdx, lbl, sizeof(lbl));
      display.setCursor(0, 18);
      display.print('['); display.print(lbl); display.print(']');

      display.drawFastHLine(0, 30, SCREEN_WIDTH, SSD1306_WHITE);

      // Static centred text (confirmed — no scrolling needed here)
      int16_t cx = (SCREEN_WIDTH - marqueeW) / 2;
      if (cx < 0) cx = 0;
      display.setCursor(cx, MARQUEE_Y);
      display.print(textBuf);   // textBuf still holds chosen option's text

      display.drawFastHLine(0, 52, SCREEN_WIDTH, SSD1306_WHITE);
      display.setCursor(20, 55);
      display.print(F("Press to continue"));
      display.display();

      if (buttonClicked()) {
        encPosition = 0;
        gameState   = STATE_PLAY_AGAIN;
      }
      break;
    }

    // --------------------------------------------------------
    case STATE_PLAY_AGAIN: {
      static const char* yesno[] = { "Play again"};
      updateEncoder(1);
      drawMenuRam("Again?", yesno, 1, encPosition);
      if (buttonClicked()) {
        if (encPosition == 0) {
          encPosition  = 0;
          lastShownOpt = 255;
          gameState    = STATE_CHOOSE_SETTING;
        } 
      }
      break;
    }
  }
}
