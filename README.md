# HomeKit Accessory Development Kit (ADK)

The HomeKit ADK is used by silicon vendors and accessory manufacturers to build HomeKit compatible devices.

The HomeKit ADK implements key components of the HomeKit Accessory Protocol (HAP), which embodies the core principles Apple brings to smart home technology: security, privacy, and reliability.

The HomeKit Open Source ADK is an open-source version of the HomeKit Accessory Development Kit. It can be used by any developer to prototype non-commercial smart home accessories. For commercial accessories, accessory developers must continue to use the commercial version of the HomeKit ADK available through the MFi Program.

Go to the [Apple Developer Site](https://developer.apple.com/homekit/) if you like to learn more about developing HomeKit-enabled accessories and apps.


### Getting Started
Please go through [Getting Started Guide](./Documentation/getting_started.md#prerequisites) before using HomeKit ADK.

### Documentation
ADK documentation is available as markdown files in [Documentation](./Documentation/) directory. However, a more user friendly
`HTML` documentation can be generated from the markdown files by running the following command:

```sh
make docs
```

The command above will prompt to open the generated HTML webpage. After the command has finished, the webpage `./Documentation/api_docs/html/index.html` can also be opened in a browser.
