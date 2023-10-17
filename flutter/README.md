The flutter front-end for the (bluetooth) DSP pipeline lives here.

The current design is a flutter plugin that has the following API:
 - connect to device (per UUID or MAC, tbd)
 - start / stop audio stream
 - control audio processing params
In native code it connects to the device, subscribes to the ble audio stream, does the audio processing and sends it to the OS audio.

This is made possible by the 'fact' that iOS and Android will allow multiple clients onto the same GATT server and multiplex them over one connection (ie, multiple apps can connect to the same BLE device, the BLE device cannot tell the difference):
 - https://stackoverflow.com/questions/37331502/can-multiple-ios-apps-on-the-same-device-connect-to-the-same-peripheral 
 - https://stackoverflow.com/questions/48428762/ios-apps-that-access-the-same-ble-peripheral-how-to-distinguish 
