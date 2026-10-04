#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define MPU6050_ADDR 0x68

// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// PUSH BUTTONS
// =====================================================

#define BUTTON_1 13
#define BUTTON_2 14
#define BUTTON_3 27
#define BUTTON_4 26

// =====================================================
// NAVIGATION VARIABLES
// =====================================================

int stepCount = 0;

int destination = 0;

// 0 = Menu
// 1 = CSE BLOCK
// 2 = ECE BLOCK
// 3 = LIBRARY
// 4 = ROOM 204

bool navigationActive = false;
bool arrived = false;

// =====================================================
// LAST REACHED LOCATION
// =====================================================

// 0 = No location reached
// 1 = CSE BLOCK
// 2 = ECE BLOCK
// 3 = LIBRARY
// 4 = ROOM 204

int lastReachedDestination = 0;

// =====================================================
// STEP DETECTION
// =====================================================

long previousAcceleration = 0;

bool stepReady = true;

unsigned long lastStepTime = 0;

// Increase this if steps happen automatically
int STEP_THRESHOLD = 3500;

// Minimum time between steps
int STEP_DELAY = 500;

// =====================================================
// TURN DETECTION
// =====================================================

unsigned long lastTurnTime = 0;

int TURN_THRESHOLD = 8000;

// =====================================================
// DESTINATION STEP COUNTS
// =====================================================

int destinationSteps[5] = {
  0,    // Menu
  20,   // CSE BLOCK
  30,   // ECE BLOCK
  40,   // LIBRARY
  50    // ROOM 204
};

// =====================================================
// FUNCTION PROTOTYPES
// =====================================================

void initializeAcceleration();

void detectStep(
  int16_t AcX,
  int16_t AcY,
  int16_t AcZ
);

void detectTurn(
  int16_t GyZ
);

void checkButtons();

void selectDestination(
  int selectedDestination
);

void waitForButtonRelease(
  int buttonPin
);

void checkArrival();

void showNavigation();

void showTurn(
  String direction
);

void showArrived();

void showAlreadyReached();

void showSelectScreen();

void printDestination();

void printDestinationOLED();

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // ===================================================
  // I2C
  // ===================================================

  Wire.begin(21, 22);

  // ===================================================
  // BUTTON SETUP
  // ===================================================

  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);

  // ===================================================
  // OLED
  // ===================================================

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDR
      )) {

    Serial.println("OLED NOT FOUND!");

    while (1);
  }

  Serial.println("OLED FOUND!");

  // ===================================================
  // START SCREEN
  // ===================================================

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(2);

  display.setCursor(
    10,
    5
  );

  display.println("SMART");

  display.setCursor(
    10,
    30
  );

  display.println("CAMPUS");

  display.display();

  delay(2000);

  // ===================================================
  // MPU6050 START
  // ===================================================

  Wire.beginTransmission(
    MPU6050_ADDR
  );

  Wire.write(0x6B);

  Wire.write(0);

  Wire.endTransmission(true);

  Serial.println(
    "MPU6050 Started"
  );

  delay(1000);

  // ===================================================
  // INITIAL MPU READING
  // ===================================================

  initializeAcceleration();

  // ===================================================
  // SHOW MENU
  // ===================================================

  showSelectScreen();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // CHECK BUTTONS
  // ===================================================

  checkButtons();

  // ===================================================
  // NO NAVIGATION
  // ===================================================

  if (!navigationActive) {

    delay(50);

    return;
  }

  // ===================================================
  // MPU6050 VARIABLES
  // ===================================================

  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;

  int16_t GyX;
  int16_t GyY;
  int16_t GyZ;

  // ===================================================
  // READ MPU6050
  // ===================================================

  Wire.beginTransmission(
    MPU6050_ADDR
  );

  Wire.write(0x3B);

  Wire.endTransmission(false);

  Wire.requestFrom(
    MPU6050_ADDR,
    14,
    true
  );

  if (
    Wire.available() < 14
  ) {

    return;
  }

  // ===================================================
  // ACCELEROMETER
  // ===================================================

  AcX =
    Wire.read() << 8 |
    Wire.read();

  AcY =
    Wire.read() << 8 |
    Wire.read();

  AcZ =
    Wire.read() << 8 |
    Wire.read();

  // ===================================================
  // SKIP TEMPERATURE
  // ===================================================

  Wire.read();
  Wire.read();

  // ===================================================
  // GYROSCOPE
  // ===================================================

  GyX =
    Wire.read() << 8 |
    Wire.read();

  GyY =
    Wire.read() << 8 |
    Wire.read();

  GyZ =
    Wire.read() << 8 |
    Wire.read();

  // ===================================================
  // STEP DETECTION
  // ===================================================

  detectStep(
    AcX,
    AcY,
    AcZ
  );

  // ===================================================
  // TURN DETECTION
  // ===================================================

  detectTurn(
    GyZ
  );

  delay(20);
}

// =====================================================
// INITIALIZE ACCELERATION
// =====================================================

void initializeAcceleration() {

  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;

  Wire.beginTransmission(
    MPU6050_ADDR
  );

  Wire.write(0x3B);

  Wire.endTransmission(false);

  Wire.requestFrom(
    MPU6050_ADDR,
    6,
    true
  );

  if (
    Wire.available() >= 6
  ) {

    AcX =
      Wire.read() << 8 |
      Wire.read();

    AcY =
      Wire.read() << 8 |
      Wire.read();

    AcZ =
      Wire.read() << 8 |
      Wire.read();

    previousAcceleration =
      sqrt(
        (long)AcX * AcX +
        (long)AcY * AcY +
        (long)AcZ * AcZ
      );
  }

  Serial.print(
    "Initial acceleration: "
  );

  Serial.println(
    previousAcceleration
  );
}

// =====================================================
// STEP DETECTION
// =====================================================

void detectStep(
  int16_t AcX,
  int16_t AcY,
  int16_t AcZ
) {

  // ===================================================
  // TOTAL ACCELERATION
  // ===================================================

  long acceleration =
    sqrt(
      (long)AcX * AcX +
      (long)AcY * AcY +
      (long)AcZ * AcZ
    );

  // ===================================================
  // CHANGE IN ACCELERATION
  // ===================================================

  long accelerationChange =
    abs(
      acceleration -
      previousAcceleration
    );

  // ===================================================
  // SAVE CURRENT ACCELERATION
  // ===================================================

  previousAcceleration =
    acceleration;

  // ===================================================
  // STEP DETECTION
  // ===================================================

  if (
    accelerationChange >
      STEP_THRESHOLD &&

    stepReady &&

    millis() -
      lastStepTime >
      STEP_DELAY
  ) {

    stepCount++;

    lastStepTime =
      millis();

    stepReady =
      false;

    Serial.print(
      "STEP: "
    );

    Serial.println(
      stepCount
    );

    // =================================================
    // CHECK ARRIVAL
    // =================================================

    checkArrival();

    if (
      !arrived &&
      navigationActive
    ) {

      showNavigation();
    }
  }

  // ===================================================
  // READY FOR NEXT STEP
  // ===================================================

  if (
    accelerationChange < 1000
  ) {

    stepReady =
      true;
  }
}

// =====================================================
// TURN DETECTION
// =====================================================

void detectTurn(
  int16_t GyZ
) {

  if (
    !navigationActive
  ) {

    return;
  }

  if (
    millis() -
    lastTurnTime <
    1200
  ) {

    return;
  }

  // ===================================================
  // RIGHT TURN
  // ===================================================

  if (
    GyZ >
    TURN_THRESHOLD
  ) {

    Serial.println(
      "TURN RIGHT"
    );

    showTurn(
      "TURN RIGHT"
    );

    lastTurnTime =
      millis();

    delay(800);

    if (
      navigationActive
    ) {

      showNavigation();
    }
  }

  // ===================================================
  // LEFT TURN
  // ===================================================

  else if (
    GyZ <
    -TURN_THRESHOLD
  ) {

    Serial.println(
      "TURN LEFT"
    );

    showTurn(
      "TURN LEFT"
    );

    lastTurnTime =
      millis();

    delay(800);

    if (
      navigationActive
    ) {

      showNavigation();
    }
  }
}

// =====================================================
// CHECK BUTTONS
// =====================================================

void checkButtons() {

  // ===================================================
  // BUTTON 1
  // ===================================================

  if (
    digitalRead(
      BUTTON_1
    ) == LOW
  ) {

    selectDestination(1);

    waitForButtonRelease(
      BUTTON_1
    );
  }

  // ===================================================
  // BUTTON 2
  // ===================================================

  else if (
    digitalRead(
      BUTTON_2
    ) == LOW
  ) {

    selectDestination(2);

    waitForButtonRelease(
      BUTTON_2
    );
  }

  // ===================================================
  // BUTTON 3
  // ===================================================

  else if (
    digitalRead(
      BUTTON_3
    ) == LOW
  ) {

    selectDestination(3);

    waitForButtonRelease(
      BUTTON_3
    );
  }

  // ===================================================
  // BUTTON 4
  // ===================================================

  else if (
    digitalRead(
      BUTTON_4
    ) == LOW
  ) {

    selectDestination(4);

    waitForButtonRelease(
      BUTTON_4
    );
  }
}

// =====================================================
// SELECT DESTINATION
// =====================================================

void selectDestination(
  int selectedDestination
) {

  // ===================================================
  // SAME AS LAST REACHED LOCATION
  // ===================================================

  if (
    selectedDestination ==
    lastReachedDestination
  ) {

    destination =
      selectedDestination;

    navigationActive =
      false;

    arrived =
      false;

    Serial.println();

    Serial.println(
      "============================"
    );

    Serial.println(
      "SORRY!"
    );

    Serial.print(
      "ALREADY REACHED: "
    );

    printDestination();

    Serial.println();

    Serial.println(
      "============================"
    );

    showAlreadyReached();

    delay(2000);

    destination = 0;

    stepCount = 0;

    showSelectScreen();

    return;
  }

  // ===================================================
  // START NEW DESTINATION
  // ===================================================

  destination =
    selectedDestination;

  stepCount = 0;

  arrived =
    false;

  navigationActive =
    true;

  // ===================================================
  // RESET STEP DETECTION
  // ===================================================

  stepReady =
    true;

  lastStepTime =
    millis();

  // ===================================================
  // RESET TURN DETECTION
  // ===================================================

  lastTurnTime =
    millis();

  // ===================================================
  // UPDATE INITIAL ACCELERATION
  // ===================================================

  initializeAcceleration();

  // ===================================================
  // SERIAL
  // ===================================================

  Serial.println();

  Serial.println(
    "============================"
  );

  Serial.print(
    "DESTINATION: "
  );

  printDestination();

  Serial.println();

  Serial.println(
    "NAVIGATION STARTED"
  );

  Serial.println(
    "============================"
  );

  // ===================================================
  // SHOW NAVIGATION
  // ===================================================

  showNavigation();

  delay(500);
}

// =====================================================
// WAIT FOR BUTTON RELEASE
// =====================================================

void waitForButtonRelease(
  int buttonPin
) {

  while (
    digitalRead(
      buttonPin
    ) == LOW
  ) {

    delay(10);
  }

  delay(100);
}

// =====================================================
// CHECK ARRIVAL
// =====================================================

void checkArrival() {

  if (
    stepCount >=
    destinationSteps[destination]
  ) {

    // =================================================
    // STOP NAVIGATION
    // =================================================

    arrived =
      true;

    navigationActive =
      false;

    // =================================================
    // SAVE ONLY LAST REACHED LOCATION
    // =================================================

    lastReachedDestination =
      destination;

    // =================================================
    // SERIAL
    // =================================================

    Serial.println();

    Serial.println(
      "============================"
    );

    Serial.println(
      "DESTINATION ARRIVED"
    );

    Serial.print(
      "LOCATION: "
    );

    printDestination();

    Serial.println();

    Serial.println(
      "============================"
    );

    // =================================================
    // ARRIVED SCREEN
    // =================================================

    showArrived();

    delay(2000);

    // =================================================
    // RESET
    // =================================================

    destination =
      0;

    stepCount =
      0;

    arrived =
      false;

    // =================================================
    // MENU
    // =================================================

    showSelectScreen();
  }
}

// =====================================================
// NAVIGATION SCREEN
// =====================================================

void showNavigation() {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  // ===================================================
  // HEADER
  // ===================================================

  display.setTextSize(1);

  display.setCursor(
    12,
    0
  );

  display.println(
    "SMART CAMPUS"
  );

  display.drawLine(
    0,
    10,
    127,
    10,
    SSD1306_WHITE
  );

  // ===================================================
  // DESTINATION
  // ===================================================

  display.setCursor(
    0,
    14
  );

  display.print(
    "TO: "
  );

  printDestinationOLED();

  // ===================================================
  // MESSAGE
  // ===================================================

  display.setTextSize(2);

  display.setCursor(
    5,
    28
  );

  if (
    stepCount <
    destinationSteps[destination] / 2
  ) {

    display.println(
      "GO STRAIGHT"
    );
  }

  else {

    display.println(
      "KEEP GOING"
    );
  }

  // ===================================================
  // STEP COUNT
  // ===================================================

  display.setTextSize(1);

  display.setCursor(
    0,
    55
  );

  display.print(
    "Steps: "
  );

  display.print(
    stepCount
  );

  display.print(
    "/"
  );

  display.print(
    destinationSteps[destination]
  );

  display.display();
}

// =====================================================
// TURN SCREEN
// =====================================================

void showTurn(
  String direction
) {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(1);

  display.setCursor(
    25,
    0
  );

  display.println(
    "NAVIGATION"
  );

  display.drawLine(
    0,
    10,
    127,
    10,
    SSD1306_WHITE
  );

  display.setTextSize(2);

  if (
    direction ==
    "TURN RIGHT"
  ) {

    display.setCursor(
      5,
      20
    );

    display.println(
      "TURN"
    );

    display.setCursor(
      5,
      42
    );

    display.println(
      "RIGHT >"
    );
  }

  else {

    display.setCursor(
      5,
      20
    );

    display.println(
      "TURN"
    );

    display.setCursor(
      5,
      42
    );

    display.println(
      "< LEFT"
    );
  }

  display.display();
}

// =====================================================
// ARRIVED SCREEN
// =====================================================

void showArrived() {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(1);

  display.setCursor(
    25,
    0
  );

  display.println(
    "NAVIGATION"
  );

  display.drawLine(
    0,
    10,
    127,
    10,
    SSD1306_WHITE
  );

  display.setTextSize(2);

  display.setCursor(
    15,
    22
  );

  display.println(
    "ARRIVED!"
  );

  display.display();
}

// =====================================================
// ALREADY REACHED SCREEN
// =====================================================

void showAlreadyReached() {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  // ===================================================
  // HEADER
  // ===================================================

  display.setTextSize(1);

  display.setCursor(
    20,
    0
  );

  display.println(
    "NAVIGATION"
  );

  display.drawLine(
    0,
    10,
    127,
    10,
    SSD1306_WHITE
  );

  // ===================================================
  // SORRY
  // ===================================================

  display.setTextSize(1);

  display.setCursor(
    42,
    17
  );

  display.println(
    "SORRY!"
  );

  // ===================================================
  // ALREADY REACHED
  // ===================================================

  display.setCursor(
    12,
    30
  );

  display.println(
    "ALREADY REACHED"
  );

  // ===================================================
  // LOCATION
  // ===================================================

  display.setCursor(
    20,
    46
  );

  printDestinationOLED();

  display.display();
}

// =====================================================
// MENU SCREEN
// =====================================================

void showSelectScreen() {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(1);

  display.setCursor(
    15,
    0
  );

  display.println(
    "SELECT LOCATION"
  );

  display.drawLine(
    0,
    10,
    127,
    10,
    SSD1306_WHITE
  );

  display.setCursor(
    0,
    15
  );

  display.println(
    "1 CSE BLOCK"
  );

  display.setCursor(
    0,
    27
  );

  display.println(
    "2 ECE BLOCK"
  );

  display.setCursor(
    0,
    39
  );

  display.println(
    "3 LIBRARY"
  );

  display.setCursor(
    0,
    51
  );

  display.println(
    "4 ROOM 204"
  );

  display.display();
}

// =====================================================
// PRINT DESTINATION TO SERIAL
// =====================================================

void printDestination() {

  if (
    destination == 1
  ) {

    Serial.print(
      "CSE BLOCK"
    );
  }

  else if (
    destination == 2
  ) {

    Serial.print(
      "ECE BLOCK"
    );
  }

  else if (
    destination == 3
  ) {

    Serial.print(
      "LIBRARY"
    );
  }

  else if (
    destination == 4
  ) {

    Serial.print(
      "ROOM 204"
    );
  }
}

// =====================================================
// PRINT DESTINATION TO OLED
// =====================================================

void printDestinationOLED() {

  if (
    destination == 1
  ) {

    display.println(
      "CSE BLOCK"
    );
  }

  else if (
    destination == 2
  ) {

    display.println(
      "ECE BLOCK"
    );
  }

  else if (
    destination == 3
  ) {

    display.println(
      "LIBRARY"
    );
  }

  else if (
    destination == 4
  ) {

    display.println(
      "ROOM 204"
    );
  }
}