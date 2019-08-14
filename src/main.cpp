/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#include "keys.h"

const char *ssid = STASSID;
const char *password = STAPSK;

const char *mqtt_user = MQTT_USER;
const char *mqtt_password = MQTT_PASS;

#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321

#define DHTPIN D2

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
PubSubClient mqtt(client);

void publish();
void wifi_connect();
void mqtt_connect();

void setup()
{
  Serial.begin(115200);
  dht.begin();
  wifi_connect();
  mqtt.setServer(MQTT_BROKER, 1883);
}

void loop()
{
  dht.begin();
  if (!mqtt.connected())
  {
    mqtt_connect();
  }
  mqtt.loop();
  publish();
  delay(300000); // execute once every 5 minutes, don't flood remote service
}

void publish()
{
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  String s;
  const int capacity = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> doc;
  doc["temperature"] = t;
  doc["humidity"] = h;
  serializeJson(doc, s);
  mqtt.publish("TiimB/feeds/sensor-test", s.c_str());
  Serial.print("Published: ");
  Serial.println(s);
}

void mqtt_connect()
{
  while (!mqtt.connected())
  {
    Serial.print("Reconnecting...");
    if (!mqtt.connect("ESP8266Client", mqtt_user, mqtt_password))
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" retrying in 5 seconds");
      delay(15000);
    }
  }
  Serial.println("  Connected");
}

void wifi_connect()
{
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}