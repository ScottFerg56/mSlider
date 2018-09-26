# mSlider
**mSlider** ('m' for 'micro') is an Arduino implementation for a DIY automated camera slider, including pan and intervalometer functions.

This project includes software, hardware designs, parts list and STL files for 3D printing some parts. The implementation, as provided, is for an [**Adafruit Feather M0 Bluefruit LE**](https://learn.adafruit.com/adafruit-feather-m0-bluefruit-le?view=all) Arduino-compatible processor but may be adapted for other suitably capable Arduino processors. The Arduino software implementation is provided
 as a [Visual Studio](https://visualstudio.microsoft.com/free-developer-offers/) solution using the [Visual Micro](https://www.visualmicro.com/) extension. There are free versions of these tools available. They are highly recommended as significantly more powerful than the Arduino IDE. The source files can probably also be built using the Arduino IDE, but no support is provided for using other tools.

This project requires other reusable libraries found in the **mLibs** companion project one level above:
* **Applet** - A uniform mechanism for organizing and communicating with hardware components
* **FMBlue** - An Applet for Bluetooth LE interfacing.
* **FMDebug** - An Applet for Serial interfacing and debugging enhancements.
* **FMIvalometer** - An Applet implementing the Intervalometer hardware functions.
* **FMStepper** - A generic Stepper Motor control Applet used to control the Slide and Pan functions.
* **FMTime** - A class to provide date/time functionality.
* **Metronome** - A simple class to provide polled Timer functions.

Place the mSlider and mLibs projects in sibling subdirectories in your file system to insure proper compilation.

The mSlider project has dependencies on several libraries, including the Arduino libraries of course:
* [**Adafruit nRF51 BLE Library**](https://learn.adafruit.com/adafruit-feather-32u4-bluefruit-le/installing-ble-library)
* [**AccelStepper library**](http://www.airspayce.com/mikem/arduino/AccelStepper/) - Required by the FMStepper Applet.
* **Feather M0 Bluefruit LE** - Check with [**Adafruit**](https://learn.adafruit.com/adafruit-feather-m0-bluefruit-le?view=all) for installing support for the board.

The **mSlider** application communicates via Bluetooth LE with a controlling application implemented in the **Slider** companion project one level above. **Slider** provides both Arduino and Windows Universal Windows Platform (UWP) implementations for controlling the Camera Slider hardare.
