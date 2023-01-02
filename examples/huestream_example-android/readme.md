# Example Android
This project is an example application on how to use the EDK on Android.

## Building the example application
Copy hue-edk-(debug/release).aar from bin to .\app\libs\hue-edk.aar
Within Android Studio: use import project -> select top level build.gradle file, and build project.

## Integrating playback in own app
### Library
The example uses the Hue Entertainment SDK library, or abbreviated as EDK.
The EDK needs to be included via the hue-edk.aar file. The archive contains a native library with generated java wrappers, so the native library needs to be loaded as can be seen in DummyExampleApplication.
Note that the library contains statically linked STL. This will not work if there are more native libraries packaged with your app, in that case we need to dynamically link STL.
The library source code and documentation can be found in the EDK repository.

### Facade
Since the EDK is rather generic, this example contains a HueHandler facade which provides a better abstraction, by only wrapping the necessary APIs and providing some additional handling for the lights synchronization use case.
It is probably a good starting point to copy HueHandler, and just adapt where needed.
Make sure to pass a short user recognizable appName for your application (instead of "Dummy Example"), so the user can be informed which app is controlling the lights.
Also make sure to pass the Application Context, and not an Activity Context, to prevent any leaking.

### User Interface
The example contains all the UI to set up and configure the Hue integration in HueSetupActivity, HueConnectActivity and HuePushlinkActivity.
It does cover all functionality, but purposely does not aim for a nice design (as that would likely not be reusable anyway).
A quick way to get a prototype up and running could be to just copy these activities, and replace them with a proper UI later on.
