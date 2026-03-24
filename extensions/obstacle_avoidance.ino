// ── EXTENSION 1: Autonomous Obstacle Avoidance ──────────────────────
// Hardware required: HC-SR04 ultrasonic sensor
//   Connect: TRIG → GPIO 13  |  ECHO → GPIO 14  |  VCC → 5V  |  GND → GND
// Add the following to the existing ESP32-S3 firmware
 
#define TRIG_PIN        13
#define ECHO_PIN        14
// Vehicle halts and avoids if obstacle detected closer than this distance
const int STOP_DISTANCE_CM = 20;
 
// getDistance() — measures distance to nearest obstacle in centimetres
// Returns 0 if echo timeout occurs (open space beyond sensor range ~5 m)
float getDistance() {
  // Send 10 µs HIGH pulse to TRIG to initiate ultrasonic burst
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Measure duration of ECHO pulse (HIGH duration proportional to distance)
  // Timeout 30000 µs = ~5 m maximum detection range
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  // Convert duration to cm: speed of sound 343 m/s, round-trip
  return duration / 58.0;
}
 
// obstacleAvoidanceMode() — autonomous stop-reverse-rotate avoidance behaviour
// Call this function in loop() when autonomous mode is active
void obstacleAvoidanceMode() {
  float dist = getDistance();
  if (dist > 0 && dist < STOP_DISTANCE_CM) {
    // Obstacle within threshold — stop, reverse briefly, rotate to clear
    stopAll();        delay(300);
    backward();       delay(500);
    stopAll();        delay(100);
    rotateRight();    delay(600);
    stopAll();
  } else {
    // Path clear — continue forward
    forward();
  }
}
 
// To integrate into existing firmware:
// In setup():  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
// In webSocketEvent(): add command handler:
//   if (cmd == "AUTO") { autoMode = true; }
//   if (cmd == "MANUAL") { autoMode = false; stopAll(); }
// In loop(): if (autoMode) { obstacleAvoidanceMode(); }
