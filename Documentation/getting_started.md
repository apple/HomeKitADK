# Getting Started

## Supported Platforms
### Darwin
#### Prerequisites
- Download and install latest [Xcode 11](https://developer.apple.com/download/more/)
- Download and install latest [Command Line Tools for XCode 11](https://developer.apple.com/download/more/)
- Download and install [Homebrew](https://brew.sh)

```sh
brew install openssl@1.1
brew install mbedtls --HEAD
brew install wget
brew install qemu
brew cask install docker
```

Run docker (Look in Spotlight/Applications folder). This is a one time instruction.
Make sure you go to Docker→Preferences→General and check the option → Start Docker Desktop when you log in

#### Compile
```sh
make all
```

#### Run
```sh
./Output/Darwin-x86_64-apple-darwin$(uname -r)/Debug/IP/Applications/Lightbulb.OpenSSL
```

### Linux
#### Prerequisites
```sh
sudo apt install docker
```

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

## Make options
Commmand                         | Description
-------------------------------- | -------------------------------------------------------------------
make ? | <ul><li>apps - Build all apps (Default)</li></li><li>test - Build unit tests</li><li>all - Build apps and unit tests</li></ul>
make APPS=? | Space delimited names of the app to compile. <br><br>Example: `make APPS=“Lightbulb Lock”`<br><br> Default: All applications
make BUILD_TYPE=? | Build type: <br><ul><li>Debug (Default)</li><li>Test</li><li>Release</li></ul>
make CRYPTO=? | Supported cryptographic libraries: <br><ul><li>OpenSSL (Default)</li><li>MbedTLS</li></ul>Example: `make CRYPTO=MbedTLS apps`
make DOCKER=? | Build with or without Docker: <br><ul><li>1 - Enable Docker during compilation (Default)</li><li>0 - Disable Docker during compilation</li></ul>
make LOG_LEVEL=level | <ul><li>0 - No logs are displayed (Default for release build)</li><li>1	- Error and Fault-level logs are displayed (Default for test build)</li><li>2 - Error, Fault-level and Info logs are displayed</li><li>3 - Error, Fault-level, Info and Debug logs are displayed (Default for debug build)</li></ul>
make PROTOCOLS=? | Space delimited protocols supported by the applications: <br><ul><li>BLE</li><li>IP</li></ul><br>Example: `make PROTOCOLS=“IP BLE”`<br><br>Default: All protocols
make TARGET=? | Build for a given target platform:<br><ul><li>Darwin</li><li>Linux</li><li>nRF52</li></li><li>Raspi</li></ul>
make USE_DISPLAY=? | Build with display support enabled:<br><ul><li>0 - Disable (Default)</li><li>1 - Enable</li></ul>
make USE_HW_AUTH=? | Build with hardware authentication enabled: <br><ul><li>0 - Disable (Default)</li><li>1 - Enable</li></ul>
make USE_NFC=? | Build with NFC enabled:<br><ul><li>0 - Disable (Default)</li><li>1 - Enable</li></ul>
make USE_WAC=? | Build with WAC enabled:<br><ul><li>0 - Disable (Default)</li><li>1 - Enable</li></ul>
