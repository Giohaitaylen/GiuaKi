#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* ssid = "Nam Mo Adidaphat 2.4G";
const char* password = "huyen299@";

// MQTT broker details
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_topic_pub = "home/lightSensor";
const char* mqtt_topic_sub = "home/ledControl";

WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin = 2;
const int lightSensorPin = 34;
int lightThreshold = 50; 

void setup_wifi() 
{
  delay(10);
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  String msg;
  for (unsigned int i = 0; i < length; i++) 
  {
    msg += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  if (String(topic) == "home/ledControl") 
  {
    if (msg == "ON") 
    {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED turned ON via MQTT");
    } 
    else if (msg == "OFF") 
    {
      digitalWrite(ledPin, LOW);
      Serial.println("LED turned OFF via MQTT");
    }
  }
}


void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) 
    {
      Serial.println("Connected to MQTT broker");
      client.subscribe("home/ledControl");
    } 
    else 
    {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  int lightValue = analogRead(lightSensorPin);
  Serial.print("Light Sensor Value: ");
  Serial.println(lightValue);

  // Publish light sensor data
  String lightString = String(lightValue);
  client.publish(mqtt_topic_pub, lightString.c_str());

  // Automatic LED control based on light sensor
  if (lightValue < lightThreshold) 
  {
    digitalWrite(ledPin, HIGH);
  } 
  else 
  {
    digitalWrite(ledPin, LOW);
  }

  delay(1000);
}
