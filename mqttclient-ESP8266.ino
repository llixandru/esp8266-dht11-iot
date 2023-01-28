#include <SimpleDHT.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "mqttclient-setup.h"

#define WIFI_CONN_DELAY 500
#define MQTT_CONN_DELAY 3000
#define DHT11_CONN_DELAY 1500

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: GPIO2
//define the digital pin used to connect the module
const int pinDHT11 = D4;

SimpleDHT11 dht11(pinDHT11);


void setup() {
  Serial.begin(115200);

  //Wi-Fi setup and connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_CONN_DELAY);
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED) Serial.println("Connected to Wi-Fi");

  //MQTT setup and connection
  //connecting to a mqtt broker
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(onMQTTConnect);
  while (!mqttClient.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s is trying to connect to the MQTT broker\n", client_id.c_str());
      if (mqttClient.connect(client_id.c_str(), MQTT_USER, MQTT_PASS)) {
          Serial.println("MQTT broker connected");
      } else {
          Serial.println("failed with state ");
          Serial.print(mqttClient.state());
          delay(MQTT_CONN_DELAY);
      }
  }
  //subscribe
  //mqttClient.subscribe(TOPIC);
}

void loop() {
  //Sensor setup
  byte temperature = 0;
  byte humidity = 0;

  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
    return;
  }
  
  //Serial.print("Sample OK: ");
  //Serial.print((int)temperature); Serial.print(" *C, "); 
  //Serial.print((int)humidity); Serial.println(" H");

  String payload = "{\"device\":\"esp8266\",\"temperature\":" + String(temperature) + ", \"humidity\":" + String(humidity) + "}";

  // publish
  mqttClient.publish(TOPIC, payload.c_str());
  
  // DHT11 sampling rate is 1HZ.
  delay(DHT11_CONN_DELAY);

  mqttClient.loop();
}

void onMQTTConnect(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

