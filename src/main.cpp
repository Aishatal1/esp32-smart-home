#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>
#include <FS.h>
#include <LittleFS.h>

// ===== WiFi Credentials =====
const char* ssid = "AhliyaStaff";
const char* password = "st@12345";

// ===== Web Server =====
WebServer server(80);

// ===== Pin Definitions =====
const int RELAY_LIVING = 14;
const int RELAY_BEDROOM = 12;
const int BUZZER_PIN = 2;  // D4 (GPIO2)
Servo servoLiving;
Servo servoBedroom;
const int SERVO_LIVING_PIN = 13;
const int SERVO_BEDROOM_PIN = 17;

const int DOORBELL_PIN = 5;

// ===== State Variables =====
bool livingLight = false;
bool bedroomLight = false;
bool livingWindowOpen = false;
bool bedroomWindowOpen = false;
bool doorbellPressed = false;
unsigned long lastDoorbellTime = 0;

// ===== Smooth Servo Movement Functions =====
void openWindowSlow(Servo &servo, int delayMs = 15) {
    for (int angle = 0; angle <= 180; angle++) {
        servo.write(angle);
        delay(delayMs);
    }
}

void closeWindowSlow(Servo &servo, int delayMs = 15) {
    for (int angle = 180; angle >= 0; angle--) {
        servo.write(angle);
        delay(delayMs);
    }
}
void ringDoorbell() {
    // Play a tone at 1000Hz for 500ms
    tone(BUZZER_PIN, 1000);
    delay(500);
    noTone(BUZZER_PIN);
}
// ===== Helper Functions =====
void sendStatus() {
  String json = "{";
  json += "\"livingLight\":" + String(livingLight ? "true" : "false") + ",";
  json += "\"bedroomLight\":" + String(bedroomLight ? "true" : "false") + ",";
  json += "\"livingWindowOpen\":" + String(livingWindowOpen ? "true" : "false") + ",";
  json += "\"bedroomWindowOpen\":" + String(bedroomWindowOpen ? "true" : "false") + ",";
  json += "\"lastDoorbell\":\"" + String(doorbellPressed ? "Just rang!" : "None") + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// ===== Setup Routes =====
void setupRoutes() {
  // Serve HTML file from LittleFS
  server.on("/", []() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Failed to load page");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });
  
  server.on("/status", sendStatus);
  
  // *** FIXED: For active-LOW relays (LOW = ON, HIGH = OFF) ***
  server.on("/living_on", []() {
    digitalWrite(RELAY_LIVING, LOW);   // LOW turns relay ON
    livingLight = true;
    server.send(200, "text/plain", "Living light ON");
  });
  
  server.on("/living_off", []() {
    digitalWrite(RELAY_LIVING, HIGH);  // HIGH turns relay OFF
    livingLight = false;
    server.send(200, "text/plain", "Living light OFF");
  });
  
  server.on("/bedroom_on", []() {
    digitalWrite(RELAY_BEDROOM, LOW);  // LOW turns relay ON
    bedroomLight = true;
    server.send(200, "text/plain", "Bedroom light ON");
  });
  
  server.on("/bedroom_off", []() {
    digitalWrite(RELAY_BEDROOM, HIGH); // HIGH turns relay OFF
    bedroomLight = false;
    server.send(200, "text/plain", "Bedroom light OFF");
  });
  
  server.on("/living_window_open", []() {
    openWindowSlow(servoLiving, 15);
    livingWindowOpen = true;
    server.send(200, "text/plain", "Living window OPEN");
  });
  
  server.on("/living_window_close", []() {
    closeWindowSlow(servoLiving, 15);
    livingWindowOpen = false;
    server.send(200, "text/plain", "Living window CLOSED");
  });
  
  server.on("/bedroom_window_close", []() {
    openWindowSlow(servoBedroom, 15);
    bedroomWindowOpen = true;
    server.send(200, "text/plain", "Bedroom window OPEN");
  });
  
  server.on("/bedroom_window_open", []() {
    closeWindowSlow(servoBedroom, 15);
    bedroomWindowOpen = false;
    server.send(200, "text/plain", "Bedroom window CLOSED");
  });
  
  server.on("/both_windows_open", []() {
    openWindowSlow(servoLiving, 15);
    openWindowSlow(servoBedroom, 15);
    livingWindowOpen = true;
    bedroomWindowOpen = true;
    server.send(200, "text/plain", "Both windows OPEN");
  });
  
  server.on("/both_windows_close", []() {
    closeWindowSlow(servoLiving, 15);
    closeWindowSlow(servoBedroom, 15);
    livingWindowOpen = false;
    bedroomWindowOpen = false;
    server.send(200, "text/plain", "Both windows CLOSED");
  });
}

// ===== Initialize LittleFS =====
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    return;
  }
  Serial.println("LittleFS mounted successfully");
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize file system
  initLittleFS();
  
  // Initialize pins
  pinMode(RELAY_LIVING, OUTPUT);
  pinMode(RELAY_BEDROOM, OUTPUT);
  digitalWrite(RELAY_LIVING, HIGH);   // Start with OFF (HIGH)
  digitalWrite(RELAY_BEDROOM, HIGH);  // Start with OFF (HIGH)
  
  servoLiving.attach(SERVO_LIVING_PIN);
  servoBedroom.attach(SERVO_BEDROOM_PIN);
  servoLiving.write(0);
  servoBedroom.write(0);
  
  pinMode(DOORBELL_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);  
  // Connect to WiFi
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
    Serial.println("❌ WiFi failed — check credentials");
  }
  
  setupRoutes();
  server.begin();
  Serial.println("HTTP server started");
}

// ===== Main Loop =====
void loop() {
  server.handleClient();
  
  // Check physical doorbell button
  if (digitalRead(DOORBELL_PIN) == LOW) {
    doorbellPressed = true;
    lastDoorbellTime = millis();
    Serial.println("🔔 DOORBELL PRESSED!");
    ringDoorbell();  
    delay(500);
  }
  
  // Reset doorbell flag after 5 seconds
  if (doorbellPressed && (millis() - lastDoorbellTime > 5000)) {
    doorbellPressed = false;
  }
}
