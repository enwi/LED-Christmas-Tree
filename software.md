# Software Guide

Compiling and uploading the software part is probably easier than the
[assembly](https://github.com/enwi/LED-Christmas-Tree/blob/main/soldering.md)
though there are still some caveats.
We provide a compiled version of the software as a [release](https://github.com/enwi/LED-Christmas-Tree/releases), which you can upload as described in [Upload release](#uploadRelease).
If you still want to compile and upload yourself you need to make sure that you comply with the requirements as described in [Compiling](#compiling).
If you already uploaded any release before you can follow the [OTA](#uploadReleaseOta) section.

## <a name="uploadRelease"></a>Upload release
Uploading a pre-built binary aka release is probably the easiest option. 
There are two ways of doing so:
The first installation needs to be done using [USB](#uploadReleaseUsb). 
If you want to update the software you can upload it wirelessly as described in the [OTA](#uploadReleaseOta) section.

### <a name="uploadReleaseUsb"></a>USB upload
1. Download and unzip [ESPEasyFlasher](https://github.com/BattloXX/ESPEasyFlasher/releases/download/1.1/FlashESP8266.zip)
2. Download the latest [release](https://github.com/enwi/LED-Christmas-Tree/releases)
3. Place `code.ino.d1_mini.bin` inside the `ESPEasyFlasher` folder
4. Using a USB cable plug in the Christmas Tree to your PC
5. Start/Execute ESPEasyFlasher
6. Select the COM-Port of your ESP
7. Select the firmware `.\code.ino.d1_mini.bin`
8. Press `Flash`
9. Enjoy your Christmas Tree

Alternatively (for example on Linux) you can use the [esptool](https://github.com/espressif/esptool) directly using python from the commandline.
For this use the command:
```
python3 esptool.py --chip esp8266 --port <your port> --baud 460800 --before default_reset --after hard_reset write_flash 0x0 code.ino.d1_mini.bin
```

### <a name="uploadReleaseOta"></a>OTA
1. Download the latest [release](https://github.com/enwi/LED-Christmas-Tree/releases)
2. Activate the AP mode of the Christmas Tree by holding down the button  until the lowest/largest ring lights up (~3 seconds)
3. Connect your phone/tablet/pc with the wifi network of the Christmas Tree
   - The Christmas Tree will show up as `LED Christmas Tree AC12B35D67EF` (the last part will differ for you as it is the MAC address)
4. Once connected your phone/tablet/pc should ask you to sign in (captive portal). On Android you will need to click on the popup on other devices (iOS, MacOS) the captive portal will open automatically. Once opened you are greeted with the OTA GUI.
5. If the page for whatever reason does not open, open a webbrowser and enter [`4.3.2.1/ota`](http://4.3.2.1/ota)
6. Press `Select file` and select the downloaded `code.ino.d1_mini.bin`.
7. Press `Upload` and wait until the browser displays `cannot load webpage`
8. Enjoy your new Christmas Tree Features

## <a name="compiling"></a>Compiling
Compiling the software yourself involves a custom setup of libraries and settings inside the Arduino IDE or VS Code.

1. Download or Clone the [repository](https://github.com/enwi/LED-Christmas-Tree)
   1. To clone run `git clone git@github.com:enwi/LED-Christmas-Tree.git` or `git clone https://github.com/enwi/LED-Christmas-Tree.git`
   2. To update the submodule run `git submodule update --init`
2. Make sure you have installed
   - [Arduino IDE](https://www.arduino.cc/en/software) version `1.8.13` or newer
   - [ESP8266 Core](https://github.com/esp8266/Arduino#contents) exactly version `2.7.4` (background https://github.com/FastLED/FastLED/issues/1322)
   - [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) version `6.18.5` or newer
   - [FastLED](https://github.com/FastLED/FastLED) version `3.4.0` or newer
   - [AceButton](https://github.com/bxparks/AceButton) version `1.9.1` or newer
   - [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) version `1.2.2` or newer
   - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) version `1.2.3` or newer
3. Open the project in Arduin0 IDE (or VSCode)
4. Select Board `LOLIN(WEMOS) D1 R2 & mini`
5. Select CPU Frequency `160 MHz` (background no flickering)
7. Using a USB cable plug in the Christmas Tree to your PC
8. Upload the software
9. Enjoy your Christmas Tree