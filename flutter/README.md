The flutter front-end for the (bluetooth) DSP pipeline lives here.

The current design is a flutter plugin that has the following API:
 - connect to device (per UUID or MAC, tbd)
 - start / stop audio stream
 - control audio processing params
   
In native code it connects to the device, subscribes to the ble audio stream, does the audio processing and sends it to the OS audio.
