![Slider Photo](https://www.forestmoon.com/Piwigo/_data/i/galleries/Projects/Slider/SEF00472-sm.jpg)

# mSlider
**mSlider** ('m' for 'micro') is an Arduino implementation for a DIY automated camera slider, including pan and intervalometer functions.

This project includes software and [plans](https://github.com/ScottFerg56/mSlider/tree/master/Plans) with a parts list and STL files for 3D printing some parts. The implementation, as provided, is for an [**Adafruit Feather M0 Bluefruit LE**](https://learn.adafruit.com/adafruit-feather-m0-bluefruit-le?view=all) Arduino-compatible processor but may be adapted for other suitably capable Arduino processors. The Arduino software implementation is provided
 as a [Visual Studio](https://visualstudio.microsoft.com/free-developer-offers/) solution using the [Visual Micro](https://www.visualmicro.com/) extension. There are free versions of these tools available. They are highly recommended as significantly more powerful than the Arduino IDE. The source files can probably also be built using the Arduino IDE, but no support is provided for using other tools.

The [**Slider gallery**](https://www.forestmoon.com/Piwigo/index.php?/category/Slider) Shows lots of construction and overview information with details in the captions. 

This project requires other reusable libraries found in the **mLibs** companion project:
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

The **mSlider** application communicates via Bluetooth LE with a controlling application implemented in the **Slider** companion project. **Slider** provides both Arduino and Windows Universal Windows Platform (UWP) implementations for controlling the Camera Slider hardare.
