#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <DHT.h>

// ======================
// DHT22
// ======================
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ======================
// PIN
// ======================
#define SERVO_PIN 18
#define POT_PIN 34
#define LED_PIN 25
#define BUZZER_PIN 26

// ======================
// LED PWM
// ======================
#define LED_CHANNEL 0
#define LED_FREQ 5000
#define LED_RESOLUTION 8

// ======================
// MQTT
// ======================
const char* mqtt_server = "192.168.1.11";
const char* mqtt_user = "rogerman";
const char* mqtt_pass = "Nainggolan123";

WiFiClient espClient;
PubSubClient client(espClient);

// ======================
// SERVO
// ======================
Servo servo;

// ======================
// MQTT CALLBACK
// ======================
void callback(char* topic, byte* payload, unsigned int length) {

  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message: ");
  Serial.println(message);

  if (String(topic) == "iotlanjut/servo") {

    if (message == "open") {

    servo.attach(SERVO_PIN);

    servo.write(90);

    Serial.println("LOCKER OPEN");

    delay(500);
    }

    else if (message == "close") {

    servo.attach(SERVO_PIN);

    servo.write(0);

    Serial.println("LOCKER CLOSE");

    delay(500);
    }
  }
}

// ======================
// MQTT RECONNECT
// ======================
void reconnect() {

  while (!client.connected()) {

    Serial.print("Connecting MQTT...");

    if (client.connect(
          "ESP32SmartDesk",
          mqtt_user,
          mqtt_pass)) {

      Serial.println("CONNECTED!");

      client.subscribe("iotlanjut/servo");

    } else {

      Serial.print("FAILED, rc=");
      Serial.println(client.state());

      delay(2000);
    }
  }
}

// ======================
// SETUP
// ======================
void setup() {

  Serial.begin(115200);

  // ======================
  // WIFI MANAGER
  // ======================

  WiFiManager wm;

  wm.autoConnect("SmartStudyDesk");

  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ======================
  // MQTT
  // ======================

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // ======================
  // DHT22
  // ======================

  dht.begin();

  delay(2000);

  // ======================
  // SERVO
  // ======================

  servo.attach(SERVO_PIN);
  servo.write(0);

  // ======================
  // BUZZER
  // ======================

  pinMode(BUZZER_PIN, OUTPUT);

  // ======================
  // LED PWM
  // ======================

  ledcSetup(
    LED_CHANNEL,
    LED_FREQ,
    LED_RESOLUTION);

  ledcAttachPin(
    LED_PIN,
    LED_CHANNEL);

  Serial.println("SMART STUDY DESK READY!");
}

// ======================
// LOOP
// ======================
void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  static unsigned long lastPublish = 0;

  if (millis() - lastPublish > 2000) {

    lastPublish = millis();

    // ======================
    // DHT22
    // ======================

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {

      // ======================
      // POTENTIOMETER
      // ======================

      int potValue = analogRead(POT_PIN);

      // ======================
      // LED PWM
      // ======================

      int brightness =
        map(potValue,
            0,
            4095,
            0,
            255);

      ledcWrite(
        LED_CHANNEL,
        brightness);

      // ======================
      // BUZZER
      // ======================

      if (temp > 40) {

        digitalWrite(
          BUZZER_PIN,
          HIGH);

      } else {

        digitalWrite(
          BUZZER_PIN,
          LOW);
      }

      // ======================
      // MQTT PUBLISH
      // ======================

      char tempString[8];
      dtostrf(
        temp,
        1,
        2,
        tempString);

      char humString[8];
      dtostrf(
        hum,
        1,
        2,
        humString);

      char potString[8];
      itoa(
        potValue,
        potString,
        10);

      client.publish(
        "iotlanjut/temp",
        tempString);

      client.publish(
        "iotlanjut/hum",
        humString);

      client.publish(
        "iotlanjut/pot",
        potString);

      // ======================
      // SERIAL MONITOR
      // ======================

      Serial.print("Temp: ");
      Serial.print(temp);

      Serial.print(" °C | Hum: ");
      Serial.print(hum);

      Serial.print(" % | Pot: ");
      Serial.print(potValue);

      Serial.print(" | Bright: ");
      Serial.println(brightness);

    } else {

      Serial.println("DHT22 Read Failed!");
    }
  }
}