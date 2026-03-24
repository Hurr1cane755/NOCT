// ================================================================
// NOCT — Nocturnal Off-road Cave Traverser
// ESP32-S3 Motor Control Firmware
// ================================================================
 
// WiFi.h             — ESP32 WiFi stack
// WebServer.h        — HTTP server: serves the browser control page on port 80
// WebSocketsServer.h — WebSocket server: receives motor commands on port 81
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
 
// WiFi credentials
const char* ssid     = "*****";
const char* password = "********";
 
// ── GPIO PIN DEFINITIONS ────────────────────────────────────────────
// IMPORTANT: Extension board label numbers do NOT match actual GPIO numbers.
// All assignments below were confirmed through systematic hardware testing.
// EN = Enable pin (PWM speed control via ledc)
// A, B = Direction pins (L298N H-bridge IN1/IN2 inputs)
 
#define LF_EN 37   // Left Front  — enable (PWM)
#define LF_A  39   // Left Front  — direction A
#define LF_B  38   // Left Front  — direction B
 
#define LR_EN 4    // Left Rear   — enable (PWM)
#define LR_A  6    // Left Rear   — direction A
#define LR_B  5    // Left Rear   — direction B
 
// Right-side motors use inverted direction logic (mirrored physical mounting)
#define RF_EN 42   // Right Front — enable (PWM)
#define RF_A  40   // Right Front — direction A
#define RF_B  41   // Right Front — direction B
 
#define RR_EN 16   // Right Rear  — enable (PWM)
#define RR_A  7    // Right Rear  — direction A
#define RR_B  15   // Right Rear  — direction B
 
WebServer         server(80);    // HTTP  — serves browser control page
WebSocketsServer  webSocket(81); // WebSocket — receives directional commands
 
// ── setAllMotors() ──────────────────────────────────────────────────
// Core motor control function. Positive PWM = forward rotation,
// negative = reverse. Direction pins are set BEFORE enabling PWM
// to prevent transient shoot-through in the H-bridge.
void setAllMotors(int lf, int lr, int rf, int rr) {
  // Zero all enable pins first for clean direction switching
  ledcWrite(LF_EN,0); ledcWrite(LR_EN,0);
  ledcWrite(RF_EN,0); ledcWrite(RR_EN,0);
  // Set direction based on sign of each PWM value
  digitalWrite(LF_A, lf>0?HIGH:LOW); digitalWrite(LF_B, lf>0?LOW:HIGH);
  digitalWrite(LR_A, lr>0?HIGH:LOW); digitalWrite(LR_B, lr>0?LOW:HIGH);
  digitalWrite(RF_A, rf>0?HIGH:LOW); digitalWrite(RF_B, rf>0?LOW:HIGH);
  digitalWrite(RR_A, rr>0?HIGH:LOW); digitalWrite(RR_B, rr>0?LOW:HIGH);
  // Brief settle time before applying PWM
  delayMicroseconds(100);
  // Apply speed — min() caps value at 255 (8-bit PWM maximum)
  ledcWrite(LF_EN,min(abs(lf),255)); ledcWrite(LR_EN,min(abs(lr),255));
  ledcWrite(RF_EN,min(abs(rf),255)); ledcWrite(RR_EN,min(abs(rr),255));
}
 
// ── MOVEMENT FUNCTIONS ──────────────────────────────────────────────
// Mecanum wheel locomotion rules:
//   Forward/backward  — all four wheels same direction
//   Strafe left/right — diagonal pairs counter-rotate
//   Rotate            — left side and right side opposite directions
// Right-side values are sign-inverted for mirrored motor orientation.
void stopAll()    { setAllMotors(   0,    0,    0,    0); }
void forward()    { setAllMotors( 200,  200, -200, -200); }
void backward()   { setAllMotors(-200, -200,  200,  200); }
void strafeLeft() { setAllMotors(-200,  200,  200, -200); }
void strafeRight(){ setAllMotors( 200, -200, -200,  200); }
void rotateLeft() { setAllMotors(-200, -200, -200, -200); }
void rotateRight(){ setAllMotors( 200,  200,  200,  200); }
 
// ── webSocketEvent() ────────────────────────────────────────────────
// Called automatically on every WebSocket event (connect, disconnect, message).
// Parses single-character command strings and calls the relevant motor function.
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  if (type == WStype_CONNECTED)
    Serial0.println("WebSocket client connected");
  if (type == WStype_TEXT) {
    String cmd = String((char*)payload);
    if      (cmd=="F")  forward();       // forward
    else if (cmd=="B")  backward();      // backward
    else if (cmd=="L")  strafeLeft();    // strafe left
    else if (cmd=="R")  strafeRight();   // strafe right
    else if (cmd=="RL") rotateLeft();    // rotate left in place
    else if (cmd=="RR") rotateRight();   // rotate right in place
    else if (cmd=="S")  stopAll();       // stop all motors
  }
}
 
// ── setup() ─────────────────────────────────────────────────────────
// Executes once on power-on. Configures GPIO pins, PWM channels,
// establishes WiFi connection, and starts both servers.
void setup() {
  Serial0.begin(115200);
  // Configure all 8 direction pins as digital outputs
  int dirPins[]={LF_A,LF_B,LR_A,LR_B,RF_A,RF_B,RR_A,RR_B};
  for(int pin:dirPins) pinMode(pin,OUTPUT);
  // Attach ledc PWM channels to enable pins (5 kHz frequency, 8-bit resolution)
  ledcAttachChannel(LF_EN,5000,8,0);
  ledcAttachChannel(LR_EN,5000,8,1);
  ledcAttachChannel(RF_EN,5000,8,2);
  ledcAttachChannel(RR_EN,5000,8,3);
  // Connect to WiFi access point — block until connected
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){delay(500);Serial0.print(".");}
  Serial0.println("\nConnected. IP: "+WiFi.localIP().toString());
  // Serve browser control page at root URL
  server.on("/",[](){server.send(200,"text/html",htmlPage);});
  server.begin();
  // Start WebSocket server and register event handler
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
 
// ── loop() ──────────────────────────────────────────────────────────
// Executes repeatedly. Polls HTTP and WebSocket servers for incoming events.
void loop() {
  server.handleClient();
  webSocket.loop();
}
