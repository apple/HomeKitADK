# HomeKit Accessory Development Kit (ADK)

The HomeKit ADK is used by silicon vendors and accessory manufacturers to build HomeKit compatible devices.

The HomeKit ADK implements key components of the HomeKit Accessory Protocol (HAP), which embodies the core principles Apple brings to smart home technology: security, privacy, and reliability.

The HomeKit Open Source ADK is an open-source version of the HomeKit Accessory Development Kit. It can be used by any developer to prototype non-commercial smart home accessories. For commercial accessories, accessory developers must continue to use the commercial version of the HomeKit ADK available through the MFi Program.

Go to the [Apple Developer Site](https://developer.apple.com/homekit/) if you like to learn more about developing HomeKit-enabled accessories and apps.

## Documentation
* [Platform Abstraction Layer](./Documentation/PAL.md)

## Darwin

#### Prerequisites
Download and install [Xcode 11](https://download.developer.apple.com/Developer_Tools/Xcode_11/Xcode_11.xip)

```sh
brew install openssl@1.1
brew install mbedtls --HEAD

```

#### Compile
```sh
make all
```

#### Run
```sh
./Output/Darwin-x86_64-apple-darwin18.6.0/Debug/IP/Applications/Lightbulb.OpenSSL
```

*NOTE:* We use the OpenSSL crypto backend by default on Darwin. You can select a different crypto module:

```sh
make CRYPTO=MbedTLS apps
```

## Linux
#### Prerequisites
```sh
brew cask install docker
```

Run docker (Look in Spotlight/Applications folder). This is a one time instruction.
Make sure you go to Docker→Preferences→General and check the option → Start Docker Desktop when you log in

#### Compile
```sh
make TARGET=Linux apps
```

## Raspberry Pi
#### Prerequisites
```sh
brew cask install docker
brew install qemu
brew install qrencode

# Run the Docker app. It is required for docker import.
./Tools/raspi_sdcard_setup.sh
```
#### Compile
```sh
make TARGET=Raspi all
```
If docker doesn't find "dev-test/raspiadk-base", run the sdcard setup and make sure to do the docker import at the end of the script.

#### Install
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
