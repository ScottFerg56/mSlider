/*

		 @@@@@    @@@      @@      @@@
		@@   @@    @@      @@       @@
		@@   @@    @@               @@
@@@ @@   @@        @@     @@@     @@@@   @@@@@  @@ @@@
@@@@@@@   @@@      @@      @@    @@ @@  @@   @@  @@  @@
@@ @ @@     @@     @@      @@   @@  @@  @@@@@@@  @@  @@
@@ @ @@ @@   @@    @@      @@   @@  @@  @@       @@
@@ @ @@ @@   @@    @@      @@   @@  @@  @@   @@  @@
@@   @@  @@@@@    @@@@    @@@@   @@@ @@  @@@@@  @@@@

 Author:	Scott Ferguson
*/

#include "ScaledStepper.h"
#include "mSlider.h"
#include "FMBlue.h"
#include "Control.h"

App	myApp;

void setup()
{
	// include the date & time of compilation in our connection banner
	debug.Init("Slider Controller\r\n" __DATE__ " " __TIME__, false, LEDpin);
	// suppress debug output when we're not heavily into debugging
//	debug.Quiet = true;
	// add the debug Applet
	myApp.AddApplet(&debug);
	// add the Applet that controls the Slider
	myApp.AddApplet(new Control('s'));
	// add the Bluetooth interface Applet
	myApp.OutputApplet = new BlueCtrl('b', "SLIDER");
	myApp.AddApplet(myApp.OutputApplet);
}

void loop()
{
	// invoke Run on all the Applets
	myApp.Run();
}
