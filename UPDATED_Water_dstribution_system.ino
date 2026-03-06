#include <LiquidCrystal_I2C.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
vo
// Ultrasonic pins
#define TRIG 8
#define ECHO 9

// Relay pins
#define PUMP 2
#define EAST 3
#define WEST 4
#define NORTH 5

// LEDs
#define LED_GREEN 10
#define LED_YELLOW 11
#define LED_RED 12

// Buttons
#define BTN_PUMP A0
#define BTN_EAST A1
#define BTN_WEST A2
#define BTN_NORTH A3

// Tank dimensions (cm)
const float tank_height = 40.0;
float level_percent = 0;

// Manual override states
bool manualPump = false;
bool manualEast = false;
bool manualWest = false;
bool manualNorth = false;

long readUltrasonic() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 25000);
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(PUMP, OUTPUT);
  pinMode(EAST, OUTPUT);
  pinMode(WEST, OUTPUT);
  pinMode(NORTH, OUTPUT);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  pinMode(BTN_PUMP, INPUT_PULLUP);
  pinMode(BTN_EAST, INPUT_PULLUP);
  pinMode(BTN_WEST, INPUT_PULLUP);
  pinMode(BTN_NORTH, INPUT_PULLUP);

  digitalWrite(PUMP, HIGH);
  digitalWrite(EAST, HIGH);
  digitalWrite(WEST, HIGH);
  digitalWrite(NORTH, HIGH);
}

void loop() {

  // Manual buttons
  if (!digitalRead(BTN_PUMP)) { manualPump = !manualPump; delay(300); }
  if (!digitalRead(BTN_EAST)) { manualEast = !manualEast; delay(300); }
  if (!digitalRead(BTN_WEST)) { manualWest = !manualWest; delay(300); }
  if (!digitalRead(BTN_NORTH)) { manualNorth = !manualNorth; delay(300); }

  // Read ultrasonic
  long distance = readUltrasonic();
  float water_height = tank_height - distance;

  if (water_height < 0) water_height = 0;
  if (water_height > tank_height) water_height = tank_height;

  level_percent = (water_height / tank_height) * 100.0;

  // -------------------------
  // ULTRASONIC ERROR CHECK
  // -------------------------
  bool ultrasonicError = false;

  if (distance == 0 || distance > tank_height) {
    ultrasonicError = true;
  }

  if (ultrasonicError) {

    // Blink RED LED
    digitalWrite(LED_RED, HIGH);
    delay(200);
    digitalWrite(LED_RED, LOW);
    delay(200);

    // Safety shutdown
    digitalWrite(PUMP, HIGH);
    digitalWrite(EAST, HIGH);
    digitalWrite(WEST, HIGH);
    digitalWrite(NORTH, HIGH);
        // Safety shutdown LEDs
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);

    // LCD Warning
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ULTRASONIC ERR");
    lcd.setCursor(0,1);
    lcd.print("Check Sensor!");

    return;  // stop loop cycle
  }

  // -------------------------
  // NORMAL LCD DISPLAY
  // -------------------------
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Level:");
  lcd.print(level_percent,1);
  lcd.print("%");

  lcd.setCursor(0,1);
  lcd.print("Pump:");
  lcd.print(manualPump ? "MAN" : "AUTO");

  // LED indications
  if (level_percent > 65) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
  }
  else if (level_percent > 35) {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, LOW);
  }
  else {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, HIGH);
  }

  // ---------------------------------------
  // AUTOMATIC VALVE + PUMP CONTROL
  // ---------------------------------------
  if (!manualPump && !manualEast && !manualWest && !manualNorth) {

    // Pump logic (corrected)
    if (level_percent <= 28) {
      digitalWrite(PUMP, LOW);   // ON
    }
    else if (level_percent >= 90) {
      digitalWrite(PUMP, HIGH);  // OFF
    }

    // Valve logic
    if (level_percent > 65) {
      digitalWrite(EAST, LOW);
      digitalWrite(WEST, LOW);
      digitalWrite(NORTH, LOW);
    }
    else if (level_percent > 50) {
      digitalWrite(EAST, LOW);
      digitalWrite(WEST, HIGH);
      digitalWrite(NORTH, LOW);
    }
    else if (level_percent > 35) {
      digitalWrite(EAST, LOW);
      digitalWrite(WEST, HIGH);
      digitalWrite(NORTH, HIGH);
    }
    else {
      digitalWrite(EAST, HIGH);
      digitalWrite(WEST, HIGH);
      digitalWrite(NORTH, HIGH);
    }
  }

  // Manual mode
  else {
    digitalWrite(PUMP, manualPump ? LOW : HIGH);
    digitalWrite(EAST, manualEast ? LOW : HIGH);
    digitalWrite(WEST, manualWest ? LOW : HIGH);
    digitalWrite(NORTH, manualNorth ? LOW : HIGH);
  }

  delay(300);
}

