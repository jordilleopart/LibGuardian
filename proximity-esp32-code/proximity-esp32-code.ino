/*
   ESP32 + HC-SR04 → MQTT
   Sends: {"sensor_id":1,"reading":12.34,"timestamp":1717861234}
   Test broker: broker.hivemq.com
   Requires:  ▸ PubSubClient
              ▸ ArduinoJson  (v6)
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>                       // for epoch timestamp

/* ───── Configuration ────────────────────────────────────────────── */
const char* ssid       = ".";           // ← your WiFi SSID
const char* password   = "123456789";   // ← your WiFi password

const char* mqtt_server = "broker.hivemq.com";  // Public test MQTT broker
const int   mqtt_port   = 1883;                  // Standard MQTT port
const char* mqtt_topic  = "libguardian/sensors/prox"; // MQTT topic to publish data

const int trigPin = 5;       // Digital pin connected to HC-SR04 trigger
const int echoPin = 18;      // Digital pin connected to HC-SR04 echo

/* ───── Global objects ────────────────────────────────────────── */
WiFiClient   espClient;            // WiFi client for TCP connection
PubSubClient client(espClient);   // MQTT client using WiFi client

/* ───── Wi-Fi + NTP ──────────────────────────────────────────────── */
void setup_wifi() {
  Serial.begin(115200);        // Initialize serial port for debug
  WiFi.mode(WIFI_STA);         // Set WiFi mode to station (client)
  WiFi.begin(ssid, password);  // Connect to WiFi network
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { // Wait until connected
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi OK  IP: " + WiFi.localIP().toString());

  /* Synchronize time (UTC) for timestamp using NTP servers */
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Synchronizing NTP");
  while (time(nullptr) < 100000) {  // Wait until time is valid
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTime ready");
}

/* ───── MQTT Connection ───────────────────────────────────────────── */
void mqtt_reconnect() {
  while (!client.connected()) {           // Keep trying until connected
    Serial.print("Connecting to MQTT…");
    if (client.connect("ESP32_HCSR04_Client")) {  // Connect with client ID
      Serial.println(" connected");
    } else {
      Serial.print(" failed rc="); Serial.print(client.state());
      Serial.println(" → retrying in 5 seconds");
      delay(5000);  // Wait before retrying connection
    }
  }
}

/* ───── HC-SR04 Sensor ──────────────────────────────────────────── */
float leerDistanciaCM() {
  // Generate pulse on trigger to start measurement
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read pulse duration on echo (microseconds)
  long dur = pulseIn(echoPin, HIGH, 30000);     // Timeout at 30 ms
  if (dur == 0)  return -1.0;                   // No echo (object not detected)
  
  // Calculate distance in cm: time * speed of sound / 2 (round trip)
  return dur * 0.034 / 2.0;
}

/* ───── setup & loop ─────────────────────────────────────────────── */
void setup() {
  pinMode(trigPin, OUTPUT);  // Set trigger pin as output
  pinMode(echoPin, INPUT);   // Set echo pin as input
  setup_wifi();              // Initialize WiFi and synchronize time

  client.setServer(mqtt_server, mqtt_port); // Configure MQTT server
}

void loop() {
  if (!client.connected()) mqtt_reconnect(); // Reconnect MQTT if needed
  client.loop();                             // Keep MQTT connection alive

  float d = leerDistanciaCM();               // Read sensor distance
  if (d <= 0) return;                        // Ignore invalid readings

  /* Build JSON safely with ArduinoJson */
  StaticJsonDocument<128> doc;
  doc["sensor_id"] = 2;                      // Sensor ID (fixed)
  doc["reading"]   = d;                      // Measured value (distance in cm)
  doc["timestamp"] = (uint32_t)time(nullptr); // Current timestamp (epoch)

  char payload[128];
  size_t n = serializeJson(doc, payload);   // Serialize JSON to string

  Serial.println(payload);                   // Print JSON to serial (debug)
  client.publish(mqtt_topic, payload, n);   // Publish JSON to MQTT topic
  delay(2000);                              // Wait 2 seconds before next reading
}