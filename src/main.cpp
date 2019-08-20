/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
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

const char *ca_cert PROGMEM = CA_CERTIFICATE;

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
PubSubClient mqtt(MQTT_BROKER, MQTT_PORT, client);

void publish();
void load_certificate();
void connect_wifi();
void connect_mqtt();
String get_device_id();

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  load_certificate();
  connect_wifi();
  dht.begin();
}

void loop()
{
  if (!mqtt.connected())
  {
    connect_mqtt();
  }
  mqtt.loop();
  publish();
  delay(300000); // execute once every 5 minutes, don't flood remote service
}

void publish()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  String s;
  const int capacity = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> doc;
  doc["temperature"] = t;
  doc["humidity"] = h;
  serializeJson(doc, s);

  String topic = "sensor/" + get_device_id();
  mqtt.publish(topic.c_str(), s.c_str());
  Serial.print("Published to ");
  Serial.println(topic);
  Serial.println(s);
}

void connect_mqtt()
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

void connect_wifi()
{
  // We start by connecting to a WiFi network
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

void load_certificate()
{
  static BearSSL::X509List ca(ca_cert);
  client.setTrustAnchors(&ca);
}

String get_device_id()
{
  String id = WiFi.macAddress();
  int idx;
  while ((idx = id.indexOf(':')) > -1)
  {
    id.remove(idx, 1);
  }
  return id.substring(0, 6);
}