# ESP32 Hydrocut Status for Siri

## What is it?

For the Trail Monitoring System, I needed to work out a HTTP REST client for the ESP32 and the Hydrocut status. In Ontario, Mountain Bike Trails are considered closed and trail maintainers would prefer you not ride when they are muddy or similar. For the Hydrocut trail system, there is a REST call one can make to determine the trail status. The backend of that monitors the Hydrocut Instagram account, and looks for different tags and reports that in a REST call. We will be using this call to allow other items to get the trail status (ie. trailhead sign). For testing purposes, I put together a Homekit device that would allow the trails status to be reported to Apple Home. Thus, allowing the question of Siri: Is the Hydrocut open?

## How this Works

There is one oddity with the Espressif homekit implementation - it does not automatically connect to the WIFI network like every other Homekit devices. Most homekit devices use Bluetooth to transfer over the WIFI credentials. The ESP version does not. This means the WIFI creds must either be hard coded into the device or the user must first use the Espressif BLE or SoftAP provisioning app on a iPhone. However, this also means that the homekit configuration is much faster as the WIFI network is already setup.

The hydrocut software uses two GPIO pins on the ESP32:
- GPIO 25 for the close LED
- GPIO 27 for the open LED

The code configures the following services that make up the garage door accessory:
- Contact Sensor (hydrocut status)

The Hydrocut status is run in the background, updated every minute, and stored in the variable allowing HomeKit to quiry the status without delay. The HTTP client is expects to take 1-5 secs to return.

## Setting up the Device

This project is based on the ESP-IDF. Sorry, no Ardino support...nor will there every be.

Make sure to run the updatemodules.sh script if you did not get the code recursively. It has a dependancy on the Espressif HomeKit SDK.

First, configure idf with menuconfig:

```bash
idf.py menuconfig
```

Make sure to set the GPIO pins to what you intend to use. Also, use the hard coded Homekit code, but make sure there are no duplicates on your network. If you use the WIFI Autoprovisioning (bluetooth version is recommended), this the Apple App Store and search for the Espressif BLE Provisioning tool. This will be required to configure WIFI.

Flash and run the monitor:

```bash
idf.py flash
idf.py monitor
```

The monitor is required for the first time to get the QR codes for WIFI provisioning and Homekit setup.

With the monitor running, you will get a screen full of data and two QR codes. The first QR code is for homekit, and the second is for the Espressif provisioning tool. Open the Espressif BLE provisioning tool, and press the Provision Device button. You will be asked to scan the second QR code (provisioning QR code). Next, enter your WIFI password. Watch the monitor. If will indicate if provisioning was succcesful as will the app. If you mess up the password, hold the boot button down for 10 seconds to wipe the configuration, and start over. Once the WIFI is working, open the Homekit app on your iPhone, and add an accessory. Scan the first QR code with Home app, and it should find the GarageDoor accessory. At this point, it's fairly quick to add the accessory. Because of all the different sensors in this device, it's a good idea to use "separate tiles" to display them. Open the setting for the garage door item in the Home app, and select the separate files item.

That is it. You can now use the Home app or Siri to control the device. The default names are ESP "item name". You can change them on your Home app.

## Resetting the ESP device

There is an button handler tied to GPIO0 which on most devices is attached to the boot button. Holding the button has two effects:
- hold for 3 seconds to reset the WIFI setup
- hold for 10 seconds to wipe the WIFI and homekit config and start over (factory default)

For a reset, one does not necessary need the monitor. You can manually add the WIFI password and manually add the device to homekit....but the monitor makes things easier.
