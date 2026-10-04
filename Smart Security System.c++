#include <IRremote.hpp>

// ---------- PINS ----------
#define TRIG_PIN    D1
#define ECHO_PIN    D2
#define SOUND_PIN   D5
#define BUZZER_PIN  D6
#define IR_PIN      D7
#define GREEN_LED   D4
#define RED_LED     D3

// ---------- SETTINGS ----------
const int DISTANCE_LIMIT = 50;  // cm

bool armed = false;
bool alarmActive = false;

// Change this after reading your remote's code
// 0 means: accept any IR button
const uint32_t ARM_BUTTON_CODE = 0;

// ------------------------------------------------

long getDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
    return 999;

  long distance = duration * 0.0343 / 2;

  return distance;
}

// ------------------------------------------------

void setDisarmed()
{
  armed = false;
  alarmActive = false;

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("SYSTEM DISARMED");
}

// ------------------------------------------------

void setArmed()
{
  armed = true;
  alarmActive = false;

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("SYSTEM ARMED");
}

// ------------------------------------------------

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(SOUND_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // Start IR receiver
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP8266 SECURITY SYSTEM");
  Serial.println("================================");
  Serial.println("System starting...");
  delay(1000);

  setDisarmed();
}

// ------------------------------------------------

void loop()
{
  // ---------- IR REMOTE ----------
  if (IrReceiver.decode())
  {
    uint32_t code = IrReceiver.decodedIRData.decodedRawData;

    Serial.print("IR Code: 0x");
    Serial.println(code, HEX);

    // If ARM_BUTTON_CODE = 0,
    // any remote button toggles the system.
    if (ARM_BUTTON_CODE == 0 || code == ARM_BUTTON_CODE)
    {
      if (armed)
        setDisarmed();
      else
        setArmed();
    }

    IrReceiver.resume();
  }

  // ---------- SECURITY CHECK ----------
  if (armed)
  {
    long distance = getDistance();

    int soundDetected = digitalRead(SOUND_PIN);

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm | Sound: ");
    Serial.println(soundDetected);

    bool objectDetected = (distance <= DISTANCE_LIMIT);

    // Many sound modules output LOW when sound is detected.
    bool loudSound = (soundDetected == LOW);

    if (objectDetected || loudSound)
    {
      alarmActive = true;

      digitalWrite(RED_LED, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);

      Serial.println("!!! INTRUSION DETECTED !!!");
    }
    else
    {
      alarmActive = false;

      digitalWrite(RED_LED, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  delay(100);
}
