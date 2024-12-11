#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
const int GREEN_BUTTON = 2;
const int YELLOW_BUTTON = 3;
const int RED_BUTTON = 4;
const int GREEN_LED = 5;
const int YELLOW_LED = 6;
const int RED_LED = 7;

// Program States
enum ProgramState {
  IDLE,
  TIMER,
  AERO_BLOOMING,
  AERO_BREWING,
  AERO_PRESSING,
  AERO_WAIT_COOLING_DOWN,
  AERO_COOLING_DOWN
};

// Global Variables
ProgramState currentState = IDLE;
unsigned long startTime = 0;  // Globalna zmienna startTime
unsigned long remainingTime = 0;
bool isRunning = false;
bool coolingDownReady = false;

void setup() {
  // Initialize pins
  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(YELLOW_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    digitalWrite(RED_LED, HIGH);
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Ready to work");
  display.display();
}

void loop() {
  // Check buttons first
  checkButtons();

  // Handle ongoing programs
  switch(currentState) {
    case TIMER:
      handleTimerProgram();
      break;
    case AERO_BLOOMING:
      handleAeroBlooming();
      break;
    case AERO_BREWING:
      handleAeroBrewing();
      break;
    case AERO_PRESSING:
      handleAeroPressing();
      break;
    case AERO_WAIT_COOLING_DOWN:
      handleAeroWaitCoolingDown();
      break;
    case AERO_COOLING_DOWN:
      handleAeroCoolingDown();
      break;
    case IDLE:
      // Ensure LEDs are off in idle state
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, LOW);
      break;
  }
}

void checkButtons() {
  // Debounce variables
  static unsigned long lastGreenButtonTime = 0;
  static unsigned long lastYellowButtonTime = 0;
  static unsigned long lastRedButtonTime = 0;
  unsigned long currentTime = millis();

  // Green Button
  if (digitalRead(GREEN_BUTTON) == LOW && currentTime - lastGreenButtonTime > 200) {
    switch(currentState) {
      case IDLE:
        startTimerProgram();
        break;
      case TIMER:
        addTimerTime();
        break;
      case AERO_WAIT_COOLING_DOWN:
        coolingDownReady = true;
        break;
      case AERO_COOLING_DOWN:
        stopProgram();
        break;
    }
    lastGreenButtonTime = currentTime;
  }

  // Yellow Button
  if (digitalRead(YELLOW_BUTTON) == LOW && currentTime - lastYellowButtonTime > 200) {
    switch(currentState) {
      case IDLE:
        startAeroProgram();
        break;
      case AERO_WAIT_COOLING_DOWN:
        coolingDownReady = true;
        break;
    }
    lastYellowButtonTime = currentTime;
  }

  // Red Button 
  if (digitalRead(RED_BUTTON) == LOW && currentTime - lastRedButtonTime > 200) {
    switch(currentState) {
      case TIMER:
        subtractTimerTime();
        break;
      default:
        stopProgram();
    }
    lastRedButtonTime = currentTime;
  }
}

void startTimerProgram() {
  currentState = TIMER;
  startTime = millis();
  remainingTime = 30000; // 30 seconds
  isRunning = true;
}

void addTimerTime() {
  if (currentState == TIMER) {
    remainingTime += 30000; // Add 30 seconds
  }
}

void subtractTimerTime() {
  if (currentState == TIMER) {
    // Ensure we don't go below 0
    if (remainingTime > 30000) {
      remainingTime -= 30000; // Subtract 30 seconds
    } else {
      remainingTime = 0; // Set to 0 if less than 30 seconds
    }
  }
}

void startAeroProgram() {
  currentState = AERO_BLOOMING;
  startTime = millis();
  remainingTime = 30000; // 30 seconds for blooming
  isRunning = true;
}

void handleTimerProgram() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  
  if (elapsedTime >= remainingTime) {
    // Timer completed
    finishTimerProgram();
    return;
  }
  
  // Blink green LED during countdown
  digitalWrite(GREEN_LED, (millis() / 500) % 2);
  
  // Calculate remaining minutes and seconds
  unsigned long remainingSeconds = (remainingTime - elapsedTime) / 1000;
  unsigned long minutes = remainingSeconds / 60;
  unsigned long seconds = remainingSeconds % 60;

  display.clearDisplay();
  display.setCursor(0,0);
  
  if (remainingSeconds < 60) {
    // If remaining time is less than 60 seconds, only show seconds
    display.print("Time: ");
    display.print(remainingSeconds);  // Display only seconds
    display.println(" sec");
  } else {
    // If remaining time is more than 60 seconds, show minutes and seconds
    display.print("Time: ");
    display.print(minutes);
    display.print(":");
    if (seconds < 10) {
      display.print("0");  // Add leading zero if less than 10
    }
    display.print(seconds);
    display.println(" min");
  }
  display.display();
}


void finishTimerProgram() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Time'u up!");
  display.display();
  
  // Wait 10 seconds
  delay(5000);
  
  // Shut down
  stopProgram();
}

void handleAeroBlooming() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  
  if (elapsedTime >= 30000) {
    // Blooming completed
    digitalWrite(YELLOW_LED, LOW);
    blinkRedLED();
    currentState = AERO_BREWING;
    startTime = millis();
    remainingTime = 90000; // 90 seconds for brewing
    return;
  }
  
  // Blink yellow LED during blooming
  digitalWrite(YELLOW_LED, (millis() / 500) % 2);
  
  // Calculate remaining minutes and seconds for blooming
  unsigned long remainingSeconds = (30000 - elapsedTime) / 1000;
  unsigned long minutes = remainingSeconds / 60;
  unsigned long seconds = remainingSeconds % 60;
  
  // Update display
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Blooming: ");
  display.print(minutes);
  display.print(":");
  if (seconds < 10) {
    display.print("0");  // Add leading zero if less than 10
  }
  display.print(seconds);
  display.println(" min");
  display.display();
}

void handleAeroBrewing() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  
  if (elapsedTime >= 90000) {
    // Brewing completed
    blinkRedLED();
    currentState = AERO_PRESSING;
    startTime = millis();
    remainingTime = 20000; // 20 seconds for pressing
    return;
  }
  
  // Blink yellow LED during brewing
  digitalWrite(YELLOW_LED, (millis() / 500) % 2);
  
  // Calculate remaining minutes and seconds for brewing
  unsigned long remainingSeconds = (90000 - elapsedTime) / 1000;
  unsigned long minutes = remainingSeconds / 60;
  unsigned long seconds = remainingSeconds % 60;
  
  // Update display
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Brewing: ");
  display.print(minutes);
  display.print(":");
  if (seconds < 10) {
    display.print("0");  // Add leading zero if less than 10
  }
  display.print(seconds);
  display.println(" min");
  display.display();
}

void handleAeroPressing() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  
  if (elapsedTime >= 20000) {
    // Pressing completed
    blinkRedLED();
    currentState = AERO_WAIT_COOLING_DOWN;
    coolingDownReady = false;
    
    // Update display
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Cooling Down?");
    display.println("Press GREEN/YELLOW");
    display.display();
    return;
  }
  
  // Blink yellow LED during pressing
  digitalWrite(YELLOW_LED, (millis() / 500) % 2);
  
  // Calculate remaining minutes and seconds for pressing
  unsigned long remainingSeconds = (20000 - elapsedTime) / 1000;
  unsigned long minutes = remainingSeconds / 60;
  unsigned long seconds = remainingSeconds % 60;
  
  // Update display
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Pressing: ");
  display.print(minutes);
  display.print(":");
  if (seconds < 10) {
    display.print("0");  // Add leading zero if less than 10
  }
  display.print(seconds);
  display.println(" min");
  display.display();
}

void handleAeroWaitCoolingDown() {
  // Wait for user confirmation to start cooling down
  if (coolingDownReady) {
    currentState = AERO_COOLING_DOWN;
    startTime = millis();
    remainingTime = 180000; // 180 seconds for cooling down
  }
}

void handleAeroCoolingDown() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  
  if (elapsedTime >= 180000) {
    // Cooling down completed
    blinkRedLED();
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Smacznej kawusi");
    display.display();
    
    // Wait 10 seconds
    delay(10000);
    
    // Shut down
    stopProgram();
    return;
  }
  
  // Blink yellow LED during cooling down
  digitalWrite(YELLOW_LED, (millis() / 500) % 2);
  
  // Calculate remaining minutes and seconds for cooling down
  unsigned long remainingSeconds = (180000 - elapsedTime) / 1000;
  unsigned long minutes = remainingSeconds / 60;
  unsigned long seconds = remainingSeconds % 60;
  
  // Update display
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Cooling Down: ");
  display.print(minutes);
  display.print(":");
  if (seconds < 10) {
    display.print("0");  // Add leading zero if less than 10
  }
  display.print(seconds);
  display.println(" min");
  display.display();
}

void blinkRedLED() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(RED_LED, HIGH);
    delay(250);
    digitalWrite(RED_LED, LOW);
    delay(250);
  }
}

void stopProgram() {
  currentState = IDLE;
  isRunning = false;
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Ready to work");
  display.display();
}
