/*

 @@@@@    @@@      @@      @@@                  
@@   @@    @@      @@       @@                  
@@   @@    @@               @@                  
 @@        @@     @@@     @@@@   @@@@@  @@ @@@  
  @@@      @@      @@    @@ @@  @@   @@  @@  @@ 
    @@     @@      @@   @@  @@  @@@@@@@  @@  @@ 
@@   @@    @@      @@   @@  @@  @@       @@     
@@   @@    @@      @@   @@  @@  @@   @@  @@     
 @@@@@    @@@@    @@@@   @@@ @@  @@@@@  @@@@    

*/

#include "ScaledStepper.h"
#include "mSlider.h"
#include "BlueCtrl.h"
#include "Control.h"

App	myApp;

void setup()
{
	debug.Init("Slider Controller\r\n" __DATE__ " " __TIME__, false, LEDpin);
	debug.Enabled = false;
	myApp.AddApplet(&debug);
	myApp.AddApplet(new Control());
	myApp.AddApplet(new BlueCtrl());
}

void loop()
{
	myApp.Run();
}
