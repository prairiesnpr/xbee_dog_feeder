Convert Mayer's best pet feeder from tuya based controls to local Zigbee.

They stopped updating their app and it no longer ran on modern phone operating systems.
That's why we don't buy cloud devices.


Requires: (https://github.com/prairiesnpr/zha_arduino)

Components:
* 1 - Teensy 3.2 - I had this in the parts bin, it's overkill, but the dual hardware serial were key
* 1 - XBeeS2C - or other XBee, I'm trying to clean out the parts bin
* 1 - A pet feeder

The command sequence I observed is documented in traffic_examples.txt

The serial logic could be improved, it works but it's naive. There are a couple status bytes that I never figured out, but I think I have the critical ones.