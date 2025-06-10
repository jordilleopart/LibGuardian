#include <SoftwareSerial.h>

#define RX_PIN      10    // RX pin for SoftwareSerial (not used here, but required)
#define TX_PIN      11    // TX pin connected to level shifter, then to OpenMote UART1 RX (J1-P6)
#define SOUND_PIN   A0    // Analog pin where Grove Sound Sensor is connected
#define SENSOR_ID   3     // Unique identifier for this sensor

// Create a SoftwareSerial object to communicate with the OpenMote
SoftwareSerial moteSerial(RX_PIN, TX_PIN);

void setup() {
  // 1) Initialize USB serial port for debugging at 115200 baud
  Serial.begin(115200);
  while(!Serial);  // Wait until Serial is ready

  // 2) Initialize SoftwareSerial port to OpenMote at 9600 baud
  moteSerial.begin(9600);

  Serial.println("Arduino ready. Sending sound data to OpenMote...");
}

void loop() {
  // 3) Read the analog value from the sound sensor (0-1023)
  int reading = analogRead(SOUND_PIN);

  // 4) Get a timestamp in milliseconds since Arduino started
  unsigned long ts = millis();

  // 5) Manually construct a JSON string with sensor_id, reading, and timestamp:
  //    Example: {"sensor_id":3,"reading":478,"timestamp":1234}
  char buf[80];
  int len = snprintf(buf, sizeof(buf),
    "{\"sensor_id\":%d,\"reading\":%d,\"timestamp\":%lu}",
    SENSOR_ID, reading, ts);

  // 6) Send the JSON string to USB Serial (for debug) and via UART1 to the OpenMote
  Serial.println(buf);                // Print JSON to USB serial monitor
  moteSerial.write((uint8_t*)buf, len);  // Send JSON bytes to OpenMote
  moteSerial.write('\n');             // Send newline to mark end of message

  // 7) Wait 1 second before the next sensor reading and send
  delay(1000);
}