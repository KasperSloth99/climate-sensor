# Introduction

This is an attempt at creating a capacitative moisture sensor based on a esp32c3 supermini.

At a high level, as seen from the esp32c3 it is quite simple:

- Generate PWM for sensor
- Measure voltage across a resistor using ADC
- Broadcast over MQTT via WIFI. Either when requsted or every minute or so - TBD

I will be using a esp32c3 supermini dev kit, which can be easily swapped out in case of faulty hardware.

It needs to compatible with [home-assistant](https://www.home-assistant.io/)

I would also like for this to use MCUboot over WIFI so i can program the sensors OTA.

## Sleep

Between broadcasts, the esp32c3 should go into deep sleep, in order to save power, as it is going to be powered by a small lithium battery.

## WIFI accessibility

The esp32c3 should store the WIFI ssid and passphrase in its flash, such that it can be changed at runtime, in case it needs to be used elsewhere. In early development, it should be set over serial, but in the future it could be set over BT LE for ease of access.
For this, the flash map from zephyr should be used.
When the MCU boots, it should read from the flash map to find the ssid and passphrase - If the connection fails, it should retry n times, and flash an LED if it fails.
