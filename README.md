# esplight-firmware
Firmware for the esplight to control it via home assistant MQTT JSON Light

Following configuration needs to be added to the configuration.yaml of home assistant for the light to work. state_topic and command_topic needs to be the same in the 
configuration of the esplights firmware.

    light:
       - platform: mqtt_json
         name: "RGB stripe"
         state_topic: "esplight1"
         command_topic: "esplight1"
         brightness: true
         rgb: true

