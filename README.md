# Share Emoji

Built for the GUITION 1.8” 360x360 ESP32-S3-JC3636W518 to share emoji between device via websocket (https://sockethook.ericbetts.dev/).

## Usage

1. Power the device, if this is your first time, connect to AutoConnectAP to setup Wifi.
2. Tap an emoji on the ring to select it.
3. When the websocket event is received, the selected emoji will be displyed in the center.

_If you selecte an emoji and your center doesn't change, it could mean the Wifi or Websocket connection is not working._


## Based on

 - [https://github.com/Skynerz/ESP32_Display_Panel-Guition_JC3636W518](Skynerz's ESP32_Display_Panel-Guition_JC3636W518)
 - PlatformIO
