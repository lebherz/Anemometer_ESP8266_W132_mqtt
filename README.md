# ESP8266_W132_mqtt

![Ventus W132](https://github.com/lebherz/ESP8266_W132_mqtt/blob/master/Ventus_W132.png?raw=true)  
Link to buy a Ventus W132: https://amzn.to/3aKWZxz

A ESP-8266 develompent board, which sents sensordata of a Ventus W132 via mqtt e.g. to HomeAssistant 
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
```
  USB-Powerdapter      ESP-8266           W132(433MHz-Board)
       USB  ------------ USB    
                         3.3V ----------------- red   
                         GND  ----------------- black
                         D7   ----------------- blue
```
![schema](https://github.com/lebherz/ESP8266_W132_mqtt/blob/master/W132_schema.png?raw=true)  
![wireing](https://github.com/lebherz/ESP8266_W132_mqtt/blob/master/W132-hack.png?raw=true)

  Your W132 does not need the batteries in its battery compartment, the W132 will get his power from the ESP-8266

orginal code is from: https://gist.github.com/micw/098709efc83a9d9ebf16d14cea4ca38e  
                      https://forum.iobroker.net/topic/23763/windanzeige-mit-ventus-w132-wemos-d1-mini/12  

## Home Assistant integration

### configuration.yaml
```
sensor:
  - platform: "mqtt"
    name: wind direction
    state_topic: "wind/<add-your-ESPID>/wind_direction"
    icon: mdi:compass-rose
    unit_of_measurement: '°'
  - platform: "mqtt"
    name: wind speed
    state_topic: "wind/<add-your-ESPID>/wind_speed"
    unit_of_measurement: 'km/h'
    value_template: '{{ (value_json|float*3.6)|round(0) }}' //value_json in meter/seconds!
    icon: mdi:speedometer
  - platform: "mqtt"
    name: wind gust
    state_topic: "wind/<add-your-ESPID>/wind_gust"
    unit_of_measurement: 'km/h'
    value_template: '{{ (value_json|float*3.6)|round(0) }}' //value_json in meter/seconds!
    icon: mdi:weather-windy
  - platform: "mqtt"
    name: wind temperature
    state_topic: "wind/<add-your-ESPID>/temp"
    icon: mdi:temperature-celsius
    unit_of_measurement: '°C'
  - platform: "mqtt"
    name: wind humity
    state_topic: "wind/<add-your-ESPID>/humi"
    icon: mdi:water-percent
    unit_of_measurement: '%'


```

### Lovelace compass-card
![wireing](https://github.com/lebherz/ESP8266_W132_mqtt/blob/master/compass-card.png?raw=true)

```
type: vertical-stack
cards:
  - compass:
      indicator: arrow_inward
      language: de
      show_north: true
    direction_offset: 0
    entity: sensor.winddirection
    name: Compass Card
    secondary_entity: sensor.windspeed
    type: 'custom:compass-card'
  - cards:
      - type: 'custom:mini-graph-card'
        entities:
          - entity: sensor.winddirection
        name: Windspeed
        font_size: 100
        line_color: var(--accent-color)
        line_width: 5
        points_per_hour: 6
      - type: 'custom:mini-graph-card'
        name: Windgust
        entities:
          - entity: sensor.windgust
        font_size: 100
        line_color: var(--accent-color)
        line_width: 5
        points_per_hour: 6
    type: horizontal-stack

```



I modified and add nessesary stuff  that it works(!)  
Have fun! René Lebherz  
