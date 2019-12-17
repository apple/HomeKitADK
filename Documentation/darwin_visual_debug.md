# Debugging on Darwin using visual debugger

## Using Visual Studio Code

### Installation
* Download VSCode: https://code.visualstudio.com/

* To install c/c++ language support: within VSCode press ⌘+p and enter
```
ext install ms-vscode.cpptools
```

* To install debugger: within VSCode, press ⌘+p and enter
```
ext install vadimcn.vscode-lldb
```
* Note: an alternative debugger is necessary because llvm no longer includes lldb-mi which the default debugger uses.


### Running an application
* Create a workspace which points to the root of your ADK folder.
* Click "Debug" > "Open Configuration", which will open the launch.json file.
* Set "type" to "lldb"
* Set "program" to the application you want to run.

Example launch.json file for Lightbulb app:
```
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "lldb",
            "request": "launch",
            "name": "Debug",
            "program": "${workspaceFolder}/Output/Darwin-x86_64-apple-darwin19.0.0/Debug/IP/Applications/Lightbulb.OpenSSL",
            "args": [],
            "cwd": "${workspaceFolder}"
        }
    ]
}
```
