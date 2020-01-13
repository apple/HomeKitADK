# HomeKit Accessory Development Kit (ADK)

The HomeKit ADK is used by silicon vendors and accessory manufacturers to build HomeKit compatible devices.

The HomeKit ADK implements key components of the HomeKit Accessory Protocol (HAP), which embodies the core principles Apple brings to smart home technology: security, privacy, and reliability.

The HomeKit Open Source ADK is an open-source version of the HomeKit Accessory Development Kit. It can be used by any developer to prototype non-commercial smart home accessories. For commercial accessories, accessory developers must continue to use the commercial version of the HomeKit ADK available through the MFi Program.

Go to the [Apple Developer Site](https://developer.apple.com/homekit/) if you like to learn more about developing HomeKit-enabled accessories and apps.

## Documentation
* Please go through [Developing with ADK](./Documentation/developing_with_adk.md) before starting development with HomeKit ADK
* [Platform Abstraction Layer](./Documentation/PAL.md)

## Darwin PAL

#### Compile
```sh
make all
```

#### Run
```sh
./Output/Darwin-x86_64-apple-darwin18.6.0/Debug/IP/Applications/Lightbulb.OpenSSL
```

## Linux PAL

#### Compile
```sh
make TARGET=Linux apps
```

## Raspberry Pi
#### Prerequisites
Run this to create the SD card image (Linux+patches) and a Docker container (your build environment).
Make sure that Docker is running before running this setup or it will fail and you will have to start over!
This will take about an hour and will stop for input many times. It will also require you to swap the card out twice.
```sh
# Run the Docker app. It is required for docker import.
./Tools/raspi_sdcard_setup.sh
```

#### Compile
```sh
make TARGET=Raspi all
```
If docker doesn't find "dev-test/raspiadk-base", run the sdcard setup and make sure to do the docker import at the end of the script.

#### Install
After building, run this to install the build products to your RaspPi. (`-n` is for the hostname of your RaspPi and `-p`
is the SSH password; both of these were chosen during the initial `raspi_sdcard_setup.sh` install).
```sh
./Tools/install.sh \
    -d raspi \
    -a Output/Raspi-armv6k-unknown-linux-gnueabihf/Debug/IP/Applications/Lightbulb.OpenSSL \
    -n raspberrypi \
    -p pi
```

#### Make options
Commmand                         | Description                                                                                       | Default
-------------------------------- | ------------------------------------------------------------------------------------------------- | -------------
make \<target\>                  | <ul><li>`apps` - Build all apps</li></li><li>`test` - Build unit tests</li><li>`all` - Build apps and unit tests</li></ul>            | all
make APPS=\<application\>        | Space delimited names of the apps to compile. Example:<br>`make APPS=“Lightbulb Lock”`                                        | All applications
make BUILD_TYPE=\<build_type\>   | Build type: <br><ul><li>`Debug`</li><li>`Test`</li><li>`Release`</ul>                                   | `Debug`
make LOG_LEVEL=\<level\>         | <ul><li>`0` - No logs are displayed</li><li>`1`	- Error and Fault-level logs are displayed</li><li>`2` - Error, Fault-level and Info logs are displayed</li><li>`3` - Error, Fault-level, Info and Debug logs are displayed</li></ul>|<ul><li>`3` - For debug build</li><li>`1` - For test build</li><li>`0` - For release build</li></ul>
make PROTOCOLS=\<protocol\>      | Space delimited protocols supported by the applications: <br><ul><li>`BLE`</li><li>`IP`</li></ul>Example: `make PROTOCOLS=“IP BLE”`                                     | All protocols
make TARGET=\<platform\>         | Build for a given target platform:<br><ul><li>`Darwin`</li><li>`Linux`</li><li>`Raspi`</li></ul>    | Build for the host Platform
make USE_HW_AUTH=\<enable\>      | Build with hardware authentication enabled: <br><ul><li>`0` - Disable</li><li>`1` - Enable</li></ul>  | Disabled
make USE_NFC=\<enable\>          | Build with NFC enabled:<br><ul><li>`0` - Disable</li><li>`1` - Enable</li></ul>                       | Disabled
