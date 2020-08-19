#define MQTT_KEEPALIVE 10

const char* MQTT_HOST = "192.168.0.2";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "xxxx";
const char* MQTT_PASSWORD = "xxxxx";

#include <PubSubClient.h>

extern const char* MQTT_FALSE = "0";
extern const char* MQTT_TRUE = "1";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

char MQTT_CLIENT_ID[15];

void setup_mqtt() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(MQTT_CLIENT_ID, "ESP8266_%02X%02X%02X", mac[3], mac[4], mac[5]);
}

void connect_mqtt(char* MQTT_TOPIC) {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
#if defined(MQTT_ALIVE_TOPIC)
    byte mac[6];
    WiFi.macAddress(mac);
    char aliveTopicId[7];
    sprintf(aliveTopicId, "%02X%02X%02X", mac[3], mac[4], mac[5]);
    char aliveTopic[255];
    sprintf(aliveTopic, MQTT_ALIVE_TOPIC, aliveTopicId);
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, aliveTopic, 1, 1, MQTT_FALSE)) {
      Serial.println("connected with last will and testament enabled");
      mqttClient.publish(aliveTopic,MQTT_TRUE,1);
      Serial.print("Sent alive message to ");
      Serial.println(aliveTopic);
#else
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
#endif
      if (MQTT_TOPIC && mqttClient.subscribe(MQTT_TOPIC,1)) {
        Serial.print("Subscribed to ");
        Serial.println(MQTT_TOPIC);
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      wdt_reset();
    }
  }
}
