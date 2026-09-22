#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============ LCD ============
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============ IR Sensors ============
const byte IR_2KM_A_PIN = 2;
const byte IR_1KM_A_PIN = 3;
const byte IR_1KM_B_PIN = 4;
const byte IR_2KM_B_PIN = 5;
const byte IR_DETECTED = LOW;

// ============ Servo ============
const byte SERVO_PIN      = 6;
const byte RED_LED_PIN    = 7;
const byte GREEN_LED_PIN  = 8;
const byte BUZZER_PIN     = 10;

const int GATE_OPEN_POS   = 90;
const int GATE_CLOSED_POS = 0;
Servo gateServo;

// ============ FSM ============
enum State {
  IDLE,
  A_TO_B_APPROACH,
  A_TO_B_NEAR,
  A_TO_B_PASSING,
  B_TO_A_APPROACH,
  B_TO_A_NEAR,
  B_TO_A_PASSING
};

State currentState = IDLE;
bool buzzerActive = false;

// ======== LCD Helper ========
void lcdPrint(const String &line1, const String &line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ======== Gate Functions ========
void closeGate() {
  for (int pos = GATE_OPEN_POS; pos >= GATE_CLOSED_POS; pos--) {
    gateServo.write(pos);
    delay(5);
  }
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);
}

void openGate() {
  for (int pos = GATE_CLOSED_POS; pos <= GATE_OPEN_POS; pos++) {
    gateServo.write(pos);
    delay(5);
  }
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(2000);
  digitalWrite(GREEN_LED_PIN, LOW);
  lcd.clear();
}

// ======== IR Sensor Check Function ========
bool trainDetected(byte sensorPin) {
  return digitalRead(sensorPin) == IR_DETECTED;
}

void setup() {
  pinMode(IR_2KM_A_PIN, INPUT);
  pinMode(IR_1KM_A_PIN, INPUT);
  pinMode(IR_1KM_B_PIN, INPUT);
  pinMode(IR_2KM_B_PIN, INPUT);

  // Outputs
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  gateServo.attach(SERVO_PIN);
  gateServo.write(GATE_OPEN_POS);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" Railway Gate ");
  lcd.setCursor(0, 1);
  lcd.print("  Monitoring  ");
  delay(2000);
  lcd.clear();

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  // Buzzer control
  digitalWrite(BUZZER_PIN, buzzerActive ? HIGH : LOW);

  switch (currentState) {
    case IDLE:
      if (trainDetected(IR_2KM_A_PIN)) {
        currentState = A_TO_B_APPROACH;
        lcdPrint("Train A->B", "2 km away");
        closeGate();
        buzzerActive = true;
      } else if (trainDetected(IR_2KM_B_PIN)) {
        currentState = B_TO_A_APPROACH;
        lcdPrint("Train B->A", "2 km away");
        closeGate();
        buzzerActive = true;
      }
      break;

    case A_TO_B_APPROACH:
      if (trainDetected(IR_1KM_A_PIN)) {
        currentState = A_TO_B_NEAR;
        lcdPrint("Train A->B", "1 km away");
      }
      break;

    case A_TO_B_NEAR:
      if (trainDetected(IR_1KM_B_PIN)) {
        currentState = A_TO_B_PASSING;
        lcdPrint("Train A->B", "Passing");
        buzzerActive = false;
      }
      break;

    case A_TO_B_PASSING:
      if (trainDetected(IR_2KM_B_PIN)) {
        lcdPrint("Train A->B", "GO");
        openGate();
        currentState = IDLE;
      }
      break;

    case B_TO_A_APPROACH:
      if (trainDetected(IR_1KM_B_PIN)) {
        currentState = B_TO_A_NEAR;
        lcdPrint("Train B->A", "1 km away");
      }
      break;

    case B_TO_A_NEAR:
      if (trainDetected(IR_1KM_A_PIN)) {
        currentState = B_TO_A_PASSING;
        lcdPrint("Train B->A", "Passing");
        buzzerActive = false;
      }
      break;

    case B_TO_A_PASSING:
      if (trainDetected(IR_2KM_A_PIN)) {
        lcdPrint("Train B->A", "GO");
        openGate();
        currentState = IDLE;
      }
      break;

    default:
      currentState = IDLE;
      buzzerActive = false;
      break;
  }
}
