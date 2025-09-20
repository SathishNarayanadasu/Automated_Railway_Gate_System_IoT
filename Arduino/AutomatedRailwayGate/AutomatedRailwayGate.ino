#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// ============ LCD ============
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============ RFID ============
#define RST_PIN   9
#define SS1_PIN   2   // Reader 1 (2 km A)
#define SS2_PIN   3   // Reader 2 (1 km A / Exit B)
#define SS3_PIN   4   // Reader 3 (1 km B / Exit A)
#define SS4_PIN   5   // Reader 4 (2 km B)

MFRC522 rfid1(SS1_PIN, RST_PIN);
MFRC522 rfid2(SS2_PIN, RST_PIN);
MFRC522 rfid3(SS3_PIN, RST_PIN);
MFRC522 rfid4(SS4_PIN, RST_PIN);

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

// ======== RFID Check Function ========
bool checkRFID(MFRC522 &rfid) {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return false;
  rfid.PICC_HaltA();  // Stop reading same card multiple times
  return true;
}

void setup() {
  // RFID
  SPI.begin();
  rfid1.PCD_Init();
  rfid2.PCD_Init();
  rfid3.PCD_Init();
  rfid4.PCD_Init();

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
      if (checkRFID(rfid1)) {
        currentState = A_TO_B_APPROACH;
        lcdPrint("Train A->B", "2 km away");
        closeGate();
        buzzerActive = true;
      } else if (checkRFID(rfid4)) {
        currentState = B_TO_A_APPROACH;
        lcdPrint("Train B->A", "2 km away");
        closeGate();
        buzzerActive = true;
      }
      break;

    case A_TO_B_APPROACH:
      if (checkRFID(rfid2)) {
        currentState = A_TO_B_NEAR;
        lcdPrint("Train A->B", "1 km away");
      }
      break;

    case A_TO_B_NEAR:
      if (checkRFID(rfid3)) {
        currentState = A_TO_B_PASSING;
        lcdPrint("Train A->B", "Passing");
        buzzerActive = false;
      }
      break;

    case A_TO_B_PASSING:
      if (checkRFID(rfid4)) {
        lcdPrint("Train A->B", "GO");
        openGate();
        currentState = IDLE;
      }
      break;

    case B_TO_A_APPROACH:
      if (checkRFID(rfid3)) {
        currentState = B_TO_A_NEAR;
        lcdPrint("Train B->A", "1 km away");
      }
      break;

    case B_TO_A_NEAR:
      if (checkRFID(rfid2)) {
        currentState = B_TO_A_PASSING;
        lcdPrint("Train B->A", "Passing");
        buzzerActive = false;
      }
      break;

    case B_TO_A_PASSING:
      if (checkRFID(rfid1)) {
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
