# ESP8266_W132_mqtt

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

  Your W132 does not need the batteries in its battery compartment, the W132 will get his power from the ESP-8266

orginal code is from: https://gist.github.com/micw/098709efc83a9d9ebf16d14cea4ca38e  
                      https://forum.iobroker.net/topic/23763/windanzeige-mit-ventus-w132-wemos-d1-mini/12  

                      I modified and add nessesary stuff  that it works(!)  
    Have fun! René Lebherz  
