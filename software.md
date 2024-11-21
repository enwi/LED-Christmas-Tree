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
3. Place `code.ino.bin` inside the `ESPEasyFlasher` folder
4. Using a USB cable plug in the Christmas Tree to your PC
5. Start/Execute ESPEasyFlasher
6. Select the COM-Port of your ESP
7. Select the firmware `.\code.ino.bin`
8. Press `Flash`
9. Enjoy your Christmas Tree

Alternatively (for example on Linux) you can use the [esptool](https://github.com/espressif/esptool) directly using python from the commandline.
For this use the command:
```
python3 esptool.py --chip esp8266 --port <your port> --baud 460800 --before default_reset --after hard_reset write_flash 0x0 code.ino.bin
```

### <a name="uploadReleaseOta"></a>OTA
1. Download the latest [release](https://github.com/enwi/LED-Christmas-Tree/releases)
2. Activate the AP mode of the Christmas Tree by holding down the button  until the lowest/largest ring lights up (~3 seconds)
3. Connect your phone/tablet/pc with the wifi network of the Christmas Tree
   - The Christmas Tree will show up as `LED Christmas Tree AC12B35D67EF` (the last part will differ for you as it is the MAC address)
4. Once connected your phone/tablet/pc should ask you to sign in (captive portal). On Android you will need to click on the popup on other devices (iOS, MacOS) the captive portal will open automatically. Once opened you are greeted with the OTA GUI.
5. If the page for whatever reason does not open, open a webbrowser and enter [`4.3.2.1/ota`](http://4.3.2.1/ota)
6. Press `Select file` and select the downloaded `code.ino.bin`.
7. Press `Upload` and wait until the browser displays `cannot load webpage`
8. Enjoy your new Christmas Tree Features

## <a name="compiling"></a>Compiling
Compiling the software yourself now uses [PlatformIO](https://platformio.org) to install and manage the required libraries automatically.

1. Install PlatformIO IDE or PlatformIO Core, depending on your preference
2. Download or Clone the [repository](https://github.com/enwi/LED-Christmas-Tree)
   1. To clone run `git clone git@github.com:enwi/LED-Christmas-Tree.git` or `git clone https://github.com/enwi/LED-Christmas-Tree.git`
   2. To update the submodule run `git submodule update --init`
3. Open the project in the PlatformIO IDE or your command line
4. Using a USB cable plug in the Christmas Tree to your PC
5. Press the upload button with the default environment (esp8266_d1_mini) or execute `pio run --target upload`
6. The build process will install all required libraries and flash the controller
7. Enjoy your Christmas Tree
