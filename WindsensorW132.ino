/**
 * A ESP-8266 develompent board, which sents sensordata of a Ventus W132 via mqtt e.g. to HomeAssistant 
   sensors:
   * wind speed 
   * wind direction 
   * wind gust 
   * temperature 
   * and huminity
   to a mqtt-broker
 
   ToDo: connect 3 wires of ESP-8266 direct to the 3 wires of the 433 MHZ-sender 
   it is not neccesary to cut the wires,
   try to stripp a bit the isolation to soldering the wires of your ESP-8266.

  USB-Powerdapter      ESP-8266           W132(433MHz-Board)
       USB  ------------ USB    
                         3.3V ----------------- red   
                         GND  ----------------- black
                         D7   ----------------- blue


  Your W132 does not need the batteries in its battery compartment, the W132 will get his power from the ESP-8266

orginal code is from: https://gist.github.com/micw/098709efc83a9d9ebf16d14cea4ca38e
                      https://forum.iobroker.net/topic/23763/windanzeige-mit-ventus-w132-wemos-d1-mini/12

                      I modified and add nessesary stuff  that it works(!)
    Have fun! René Lebherz
    https://github.com/lebherz/ESP8266_W132_mqtt
 */
#define MQTT_ALIVE_TOPIC "wind/%s/alive"
#define MQTT_DATA_TOPIC "wind/%s/%s"

#include "common_wifi.h"
#include "common_mqtt.h"
//#include <esp_pins.h>

#define PIN_ANEMOMETER 13 // GPIO13 or D7

volatile unsigned long lastTrigger;

byte mac[6];
char sensorId[7];
char topicBuffer[255];
char messageBuffer[255];

byte bitPos=-1;
byte messageNum=-1;
// there are always sent 6 messages. For temperature/humidity, the message is repeatet 6 times. For wind, 2 messages are repeatet 3 times each
int message1Bits[36];
int message2Bits[36];

bool verifyChecksum(int bits[]) {
  int checksum=0xf;
  for (int i=0;i<8;i++) {
    checksum-=bits[i*4]|bits[i*4+1]<<1|bits[i*4+2]<<2|bits[i*4+3]<<3;
  }
  checksum&=0xf;
  int expectedChecksum=bits[32]|bits[33]<<1|bits[34]<<2|bits[35]<<3;
  return checksum==expectedChecksum;
}

void decodeMessages() {
  if (!verifyChecksum(message1Bits)) {
    Serial.println("Checksum mismatch in message #1");
    return;
  }
  
  if (message1Bits[9]==1 && message1Bits[10]==1) { // wind data (2 messages)

    if (!verifyChecksum(message2Bits)) {
      Serial.println("Checksum mismatch in message #1");
      return;
    }
    
    float windSpeed=(message1Bits[24]    | message1Bits[25]<<1 | message1Bits[26]<<2 | message1Bits[27]<<3 |
                     message1Bits[28]<<4 | message1Bits[29]<<5 | message1Bits[30]<<6 | message1Bits[31]<<7)*0.2f;
    Serial.print("Average wind speed: ");
    Serial.print(windSpeed);
    Serial.println(" m/s");

    sprintf(topicBuffer, MQTT_DATA_TOPIC, sensorId, "wind_speed");
    String value = String(windSpeed,1);
    value.toCharArray(messageBuffer,255);
    mqttClient.publish(topicBuffer,(byte*)messageBuffer,strlen(messageBuffer));


    float windGust=(message2Bits[24]    | message2Bits[25]<<1 | message2Bits[26]<<2 | message2Bits[27]<<3 |
                    message2Bits[28]<<4 | message2Bits[29]<<5 | message2Bits[30]<<6 | message2Bits[31]<<7)*0.2f;
    Serial.print("Max wind speed: ");
    Serial.print(windGust);
    Serial.println(" m/s");

    sprintf(topicBuffer, MQTT_DATA_TOPIC, sensorId, "wind_gust");
    value = String(windGust,1);
    value.toCharArray(messageBuffer,255);
    mqttClient.publish(topicBuffer,(byte*)messageBuffer,strlen(messageBuffer));

    int windDirection=(message2Bits[15]    | message2Bits[16]<<1 | message2Bits[17]<<2 | message2Bits[18]<<3 |
                       message2Bits[19]<<4 | message2Bits[20]<<5 | message2Bits[21]<<6 | message2Bits[22]<<7 |
                       message2Bits[23]<<8);
    Serial.print("Wind direction: ");
    Serial.print(windDirection);
    Serial.println(" °");

    sprintf(topicBuffer, MQTT_DATA_TOPIC, sensorId, "wind_direction");
    value = String(windDirection);
    value.toCharArray(messageBuffer,255);
    mqttClient.publish(topicBuffer,(byte*)messageBuffer,strlen(messageBuffer));
    
  } else {  // temperature/humidity in both messages
    int temperatureRaw=(message1Bits[12]    | message1Bits[13]<<1 | message1Bits[14]<<2 | message1Bits[15]<<3 |
                        message1Bits[16]<<4 | message1Bits[17]<<5 | message1Bits[18]<<6 | message1Bits[19]<<7 | 
                        message1Bits[20]<<8 | message1Bits[21]<<9 | message1Bits[22]<<10| message1Bits[23]<<11);
    if (temperatureRaw& 0x800) temperatureRaw+=0xF000; // negative number, 12 to 16 bit
    float temperature=temperatureRaw*0.1f;
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    sprintf(topicBuffer, MQTT_DATA_TOPIC, sensorId, "temp");
    String value = String(temperature,1);
    value.toCharArray(messageBuffer,255);
    mqttClient.publish(topicBuffer,(byte*)messageBuffer,strlen(messageBuffer));
    
    int humidity=(message1Bits[24] | message1Bits[25]<<1 | message1Bits[26]<<2 | message1Bits[27]<<3 )+
                 (message1Bits[28] | message1Bits[29]<<1 | message1Bits[30]<<2 | message1Bits[31]<<3 )*10;
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    sprintf(topicBuffer, MQTT_DATA_TOPIC, sensorId, "humi");
    value = String(humidity);
    value.toCharArray(messageBuffer,255);
    mqttClient.publish(topicBuffer,(byte*)messageBuffer,strlen(messageBuffer));
    
  }
  
}

ICACHE_RAM_ATTR void dataTrigger() {
  unsigned long now=micros();
  unsigned long duration=now-lastTrigger;
  lastTrigger=now;

  if (duration>30000) { // a news block of messages begins
    messageNum=0;
  }

  if (duration>7000) { // ~9 ms = sync signal
    if (bitPos==36) { // we got a full message
      if (messageNum==0) { // 1st message completed
        messageNum=1;
      } else if (messageNum==1) { // 2nd message completed
        decodeMessages();
        messageNum=-1;
      }
    }
    bitPos=0; // message beginns
    return;
  }

  if (messageNum<0) return; // ignore repeated messages
  if (bitPos<0) return; // invalid message, ignored

  if (messageNum==0) {
    message1Bits[bitPos]=(duration>3300); // 2.2ms=LOW, 4.4ms = HIGH bits
  } else {
    message2Bits[bitPos]=(duration>3300); // 2.2ms=LOW, 4.4ms = HIGH bits
  }
  bitPos++;
  if (bitPos>36) bitPos=-1; // message too long -> invalid
}

void setup() {
  wdt_enable(WDTO_8S); // reset watchdog every 8 seconds
  Serial.begin(115200);
  Serial.println("Initializing");

  WiFi.macAddress(mac);
  sprintf(sensorId, "%02X%02X%02X", mac[3], mac[4], mac[5]);

  setup_wifi();
  wdt_reset(); // reset watchdog 
  setup_mqtt();
  wdt_reset(); // reset watchdog 
  
  pinMode(PIN_ANEMOMETER,INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ANEMOMETER), dataTrigger, FALLING);

  wdt_reset(); // reset watchdog 

  Serial.println("Starting");
}

void loop() {
  wdt_reset(); // Watchdog resetten
  connect_mqtt(0);
  wdt_reset(); // Watchdog resetten
  mqttClient.loop();
}
