/*

         OOOOO    OOO      OO      OOO
        OO   OO    OO      OO       OO
        OO   OO    OO               OO
OOO OO   OO        OO     OOO     OOOO   OOOOO  OO OOO
OOOOOOO   OOO      OO      OO    OO OO  OO   OO  OO  OO
OO O OO     OO     OO      OO   OO  OO  OOOOOOO  OO  OO
OO O OO OO   OO    OO      OO   OO  OO  OO       OO
OO O OO OO   OO    OO      OO   OO  OO  OO   OO  OO
OO   OO  OOOOO    OOOO    OOOO   OOO OO  OOOOO  OOOO

    (c) 2018 Scott Ferguson
    This code is licensed under MIT license (see LICENSE file for details)

    An Arduino implementation for a DIY automated camera slider, including pan and intervalometer functions.
    For more information, visit: https://github.com/ScottFerg56/mSlider/tree/master.
*/

#include "mSlider.h"
#include <FMBlue.h>
#include <FMStepper.h>
#include <FMIvalometer.h>
#include "Global.h"

App	myApp;

void setup()
{
	// include the date & time of compilation in our connection banner
	fmDebug.Init("Slider Controller\r\n" __DATE__ " " __TIME__, false, LEDpin);
	// suppress debug output when we're not heavily into debugging
//	fmDebug.Quiet = true;
	// add the debug Applet
	myApp.AddApplet(&fmDebug);

	// add the stepper Applet that controls the Slider
	AccelStepper* stepper = new AccelStepper(AccelStepper::DRIVER, SlideStep_Pin, SlideDir_Pin);
	FMStepper* fmstepper = new FMStepper('s', 80.0, stepper, SlideLimit_Pin);
	stepper->setPinsInverted(true);
	fmstepper->SetSpeedLimit(50);
	fmstepper->SetMaxSpeed(30);
	fmstepper->SetAcceleration(25);
	fmstepper->SetLimits(0, 640);
	fmstepper->Name = "Slide";
	myApp.AddApplet(fmstepper);

	// add the stepper Applet that controls the Pan
	stepper = new AccelStepper(AccelStepper::DRIVER, PanStep_Pin, PanDir_Pin);
	fmstepper = new FMStepper('p', 400.0 / 9, stepper);
	fmstepper->SetSpeedLimit(90);
	fmstepper->SetMaxSpeed(55);
	fmstepper->SetAcceleration(45);
	fmstepper->SetLimits(-360, 360);
	fmstepper->Name = "Pan";
	myApp.AddApplet(fmstepper);

	FMIvalometer* ivalometer = new FMIvalometer('i', Focus_Pin, Shutter_Pin);
	ivalometer->Name = "Intervalometer";
	myApp.AddApplet(ivalometer);

	myApp.AddApplet(new Global('g'));

	// add the Bluetooth interface Applet
	myApp.OutputApplet = new FMBlue('b', "SLIDER");
	myApp.AddApplet(myApp.OutputApplet);
}

void loop()
{
	// invoke Run on all the Applets
	myApp.Run();
}
