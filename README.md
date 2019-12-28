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
##### Make options
All of the feature items below are disabled by default

To build with Hardware Authentication
```sh
make USE_HW_AUTH=1 all
```

To build with NFC enabled
```sh
make USE_NFC=1 all
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
