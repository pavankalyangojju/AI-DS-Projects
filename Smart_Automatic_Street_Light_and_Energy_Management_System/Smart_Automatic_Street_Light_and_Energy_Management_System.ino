// =====================================================
// SMART AUTOMATIC STREET LIGHT
// AND ENERGY MANAGEMENT SYSTEM
// Arduino UNO
// =====================================================

// ---------------- PIN DEFINITIONS ----------------

#define LDR_PIN       A0
#define VOLTAGE_PIN   A1
#define CURRENT_PIN   A2

#define PIR_PIN       2

#define LED1_PIN      3
#define LED2_PIN      5
#define LED3_PIN      6
#define LED4_PIN      9

// ---------------- LDR SETTINGS ----------------

// Change this value according to your LDR
int darkThreshold = 500;

// ---------------- VOLTAGE SENSOR ----------------

// 0-25V Voltage Sensor Module
// Voltage divider ratio is approximately 5:1
float voltageFactor = 5.0;

// ---------------- ACS712 SETTINGS ----------------

// Select according to your ACS712 module:
//
// ACS712 5A  -> 0.185 V/A
// ACS712 20A -> 0.100 V/A
// ACS712 30A -> 0.066 V/A

float sensitivity = 0.185;

// ACS712 zero-current voltage
float zeroCurrentVoltage = 2.5;


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  pinMode(LDR_PIN, INPUT);
  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(CURRENT_PIN, INPUT);

  pinMode(PIR_PIN, INPUT);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);

  // Initially turn OFF all lights

  analogWrite(LED1_PIN, 0);
  analogWrite(LED2_PIN, 0);
  analogWrite(LED3_PIN, 0);
  analogWrite(LED4_PIN, 0);

  Serial.println("=================================");
  Serial.println(" SMART STREET LIGHT SYSTEM");
  Serial.println(" ENERGY MANAGEMENT SYSTEM");
  Serial.println("=================================");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ---------------- READ SENSORS ----------------

  int ldrValue = analogRead(LDR_PIN);

  int pirValue = digitalRead(PIR_PIN);

  // Read voltage
  int voltageRaw = analogRead(VOLTAGE_PIN);

  float sensorVoltage =
      voltageRaw * (5.0 / 1023.0);

  float supplyVoltage =
      sensorVoltage * voltageFactor;


  // ---------------- READ CURRENT ----------------

  int currentRaw = analogRead(CURRENT_PIN);

  float currentSensorVoltage =
      currentRaw * (5.0 / 1023.0);

  float current =
      (currentSensorVoltage - zeroCurrentVoltage)
      / sensitivity;


  // Remove small negative readings

  if (current < 0.05) {
    current = 0;
  }


  // ---------------- POWER CALCULATION ----------------

  float power = supplyVoltage * current;


  // =================================================
  // DAY / NIGHT DETECTION
  // =================================================

  if (ldrValue < darkThreshold) {

    // =================================================
    // NIGHT
    // =================================================

    if (pirValue == HIGH) {

      // -----------------------------------------------
      // NIGHT + MOTION
      // Full brightness
      // -----------------------------------------------

      analogWrite(LED1_PIN, 255);
      analogWrite(LED2_PIN, 255);
      analogWrite(LED3_PIN, 255);
      analogWrite(LED4_PIN, 255);

      Serial.println("NIGHT + MOTION DETECTED");
      Serial.println("Lights: FULL BRIGHTNESS");

    }

    else {

      // -----------------------------------------------
      // NIGHT + NO MOTION
      // Energy saving mode
      // -----------------------------------------------

      analogWrite(LED1_PIN, 80);
      analogWrite(LED2_PIN, 80);
      analogWrite(LED3_PIN, 80);
      analogWrite(LED4_PIN, 80);

      Serial.println("NIGHT + NO MOTION");
      Serial.println("Energy Saving Mode");

    }

  }

  else {

    // =================================================
    // DAY
    // =================================================

    analogWrite(LED1_PIN, 0);
    analogWrite(LED2_PIN, 0);
    analogWrite(LED3_PIN, 0);
    analogWrite(LED4_PIN, 0);

    Serial.println("DAYTIME");
    Serial.println("Street Lights: OFF");
  }


  // =================================================
  // DISPLAY SENSOR VALUES
  // =================================================

  Serial.println("---------------------------------");

  Serial.print("LDR Value       : ");
  Serial.println(ldrValue);

  Serial.print("PIR Status      : ");

  if (pirValue == HIGH) {
    Serial.println("MOTION");
  }
  else {
    Serial.println("NO MOTION");
  }

  Serial.print("Voltage         : ");
  Serial.print(supplyVoltage, 2);
  Serial.println(" V");

  Serial.print("Current         : ");
  Serial.print(current, 2);
  Serial.println(" A");

  Serial.print("Power           : ");
  Serial.print(power, 2);
  Serial.println(" W");

  Serial.println("---------------------------------");

  delay(1000);
}