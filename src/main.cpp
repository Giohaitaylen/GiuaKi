#include <Arduino.h>
#include "secrets/wifi.h"
#include "wifi_connect.h"
#include <WiFiClientSecure.h>
#include "ca_cert.h"
#include <PubSubClient.h>
#include "secrets/mqtt.h"
#include <Ticker.h>

namespace
{
    const char *ssid = WiFiSecrets::ssid;
    const char *password = WiFiSecrets::pass;
    const char *echo_topic = "esp32/echo_test";
    unsigned int publish_count = 0;
    uint16_t keepAlive = 15;    // seconds (default is 15)
    uint16_t socketTimeout = 5; // seconds (default is 15)
}

WiFiClientSecure tlsClient;
PubSubClient mqttClient(tlsClient);

Ticker mqttPulishTicker;

const int ledPin = 2;              // GPIO2 (Built-in LED on some ESP32 boards)
const int lightSensorPin = 34;     // Analog input pin for light sensor

int lightThreshold = 50;

void mqttPublish()
{
    Serial.print("Publishing: ");
    Serial.println(publish_count);
    mqttClient.publish(echo_topic, String(publish_count).c_str(), false);
    publish_count++;
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("From %s:  ", topic);
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void mqttReconnect()
{
    while (!mqttClient.connected())
    {
        Serial.println("Attempting MQTT connection...");
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        if (mqttClient.connect(client_id.c_str(), MQTT::mqtt_user, MQTT::mqtt_password))
        {
            Serial.print(client_id);
            Serial.println(" connected");
            mqttClient.subscribe(echo_topic);
        }
        else
        {
            Serial.print("MTTT connect failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 1 seconds");
            delay(1000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); 
    setup_wifi(ssid, password);
    tlsClient.setCACert(ca_cert);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setServer(MQTT::mqtt_server, MQTT::mqtt_port);
    mqttPulishTicker.attach(1, mqttPublish);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
     Serial.println("WiFi disconnected, attempting reconnection");
     setup_wifi();
     }

  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();

  // Read the light sensor value
  int lightValue = analogRead(lightSensorPin);
  Serial.print("Light Sensor Value: ");
  Serial.println(lightValue);

  // Publish light sensor data
  String lightString = String(lightValue);
  mqttClient.publish(MQTT::mqtt_topic_pub, lightString.c_str());

  // Automatic LED control based on light sensor value
  if (lightValue < lightThreshold) {
    digitalWrite(ledPin, HIGH); // Turn on LED
  } else {
    digitalWrite(ledPin, LOW);  // Turn off LED
  }

  delay(2000); // Adjust the delay as needed
}