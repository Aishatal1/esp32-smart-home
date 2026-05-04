#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// ===== WiFi Credentials =====
const char* ssid = "Mansour";
const char* password = "mantal@2026";

// ===== Pin Definitions =====
const int RELAY_LIVING = 14;   // D5
const int RELAY_BEDROOM = 12;  // D6

Servo servoLiving;
Servo servoBedroom;
const int SERVO_LIVING_PIN = 13;   // D7
const int SERVO_BEDROOM_PIN = 17;  // GPIO17

const int DOORBELL_PIN = 5;   // D1

// ===== State Variables =====
bool livingLight = false;
bool bedroomLight = false;
bool livingWindowOpen = false;
bool bedroomWindowOpen = false;

// ===== Function Declarations =====
void printMenu();
void printStatus();
void printWiFiStatus();
void executeCommand(int cmd);

// ===== Print WiFi Status =====
void printWiFiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("📶 WiFi: CONNECTED | IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("   Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("📶 WiFi: DISCONNECTED");
  }
}

// ===== Print Status =====
void printStatus() {
  Serial.println("\n========== SMART HOME STATUS ==========");
  
  Serial.print("1. Living Room Light: ");
  Serial.println(livingLight ? "ON 💡" : "OFF ⚫");
  
  Serial.print("2. Bedroom Light: ");
  Serial.println(bedroomLight ? "ON 💡" : "OFF ⚫");
  
  Serial.print("3. Living Room Window: ");
  Serial.println(livingWindowOpen ? "OPEN 🪟" : "CLOSED ❌");
  
  Serial.print("4. Bedroom Window: ");
  Serial.println(bedroomWindowOpen ? "OPEN 🪟" : "CLOSED ❌");
  
  printWiFiStatus();
  Serial.println("========================================\n");
}

// ===== Menu =====
void printMenu() {
  Serial.println("\n========== SMART HOME CONTROL ==========");
  Serial.println("🔹 LIGHTS:");
  Serial.println("  11 - Living Room ON");
  Serial.println("  12 - Living Room OFF");
  Serial.println("  21 - Bedroom ON");
  Serial.println("  22 - Bedroom OFF");
  Serial.println();
  Serial.println("🔹 WINDOWS:");
  Serial.println("  31 - Living Window OPEN");
  Serial.println("  32 - Living Window CLOSE");
  Serial.println("  41 - Bedroom Window OPEN");
  Serial.println("  42 - Bedroom Window CLOSE");
  Serial.println("  50 - BOTH Windows OPEN");
  Serial.println("  51 - BOTH Windows CLOSE");
  Serial.println();
  Serial.println("🔹 INFO:");
  Serial.println("  99 - Show Status");
  Serial.println("  98 - Show WiFi Status");
  Serial.println("  0  - Show this Menu");
  Serial.println("========================================\n");
}

// ===== Execute Command =====
void executeCommand(int cmd) {
  Serial.println();
  
  switch(cmd) {
    // Living Room Light
    case 11:
      digitalWrite(RELAY_LIVING, HIGH);
      livingLight = true;
      Serial.println("💡 Living Room Light: ON");
      break;
    case 12:
      digitalWrite(RELAY_LIVING, LOW);
      livingLight = false;
      Serial.println("💡 Living Room Light: OFF");
      break;
      
    // Bedroom Light
    case 21:
      digitalWrite(RELAY_BEDROOM, HIGH);
      bedroomLight = true;
      Serial.println("💡 Bedroom Light: ON");
      break;
    case 22:
      digitalWrite(RELAY_BEDROOM, LOW);
      bedroomLight = false;
      Serial.println("💡 Bedroom Light: OFF");
      break;
      
    // Living Window
    case 31:
      servoLiving.write(180);
      livingWindowOpen = true;
      Serial.println("🪟 Living Room Window: OPEN");
      break;
    case 32:
      servoLiving.write(0);
      livingWindowOpen = false;
      Serial.println("🪟 Living Room Window: CLOSED");
      break;
      
    // Bedroom Window
    case 41:
      servoBedroom.write(180);
      bedroomWindowOpen = true;
      Serial.println("🪟 Bedroom Window: OPEN");
      break;
    case 42:
      servoBedroom.write(0);
      bedroomWindowOpen = false;
      Serial.println("🪟 Bedroom Window: CLOSED");
      break;
      
    // Both Windows
    case 50:
      servoLiving.write(180);
      servoBedroom.write(180);
      livingWindowOpen = true;
      bedroomWindowOpen = true;
      Serial.println("🪟🪟 BOTH Windows: OPEN");
      break;
    case 51:
      servoLiving.write(0);
      servoBedroom.write(0);
      livingWindowOpen = false;
      bedroomWindowOpen = false;
      Serial.println("🪟🪟 BOTH Windows: CLOSED");
      break;
      
    // Info
    case 99:
      printStatus();
      break;
    case 98:
      printWiFiStatus();
      break;
    case 0:
      printMenu();
      break;
      
    default:
      Serial.print("❌ Unknown command: ");
      Serial.println(cmd);
      Serial.println("Type 0 for menu");
      break;
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Initialize pins
  pinMode(RELAY_LIVING, OUTPUT);
  pinMode(RELAY_BEDROOM, OUTPUT);
  digitalWrite(RELAY_LIVING, LOW);
  digitalWrite(RELAY_BEDROOM, LOW);
  
  servoLiving.attach(SERVO_LIVING_PIN);
  servoBedroom.attach(SERVO_BEDROOM_PIN);
  servoLiving.write(0);
  servoBedroom.write(0);
  
  pinMode(DOORBELL_PIN, INPUT_PULLUP);
  
  // Connect to WiFi with timeout
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("✅ WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("❌ WiFi FAILED — check credentials or network");
  }
  
  printMenu();
}

// ===== Main Loop =====
void loop() {
  // Check doorbell
  if (digitalRead(DOORBELL_PIN) == LOW) {
    Serial.println("\n🔔🔔🔔 SOMEONE IS AT THE DOOR! 🔔🔔🔔\n");
    delay(500);
  }
  
  // Read user input (waits for full line)
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() > 0) {
      int command = input.toInt();
      executeCommand(command);
    }
  }
}
