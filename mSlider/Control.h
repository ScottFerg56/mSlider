/*

  @@@@                     @                      @@@
 @@  @@                   @@                       @@
@@    @                   @@                       @@
@@       @@@@@  @@ @@@  @@@@@@  @@ @@@   @@@@@     @@
@@      @@   @@  @@@@@@   @@     @@  @@ @@   @@    @@
@@      @@   @@  @@  @@   @@     @@  @@ @@   @@    @@
@@    @ @@   @@  @@  @@   @@     @@     @@   @@    @@
 @@  @@ @@   @@  @@  @@   @@ @@  @@     @@   @@    @@
  @@@@   @@@@@   @@  @@    @@@  @@@@     @@@@@    @@@@

*/

/*
An applet for controlling the stepper motors for the camera slider/panner.
*/

#ifndef Control_h
#define Control_h

#include "mSlider.h"
#include "ScaledStepper.h"

class Control : public Applet
{
public:
	Control() : Timer(500), SlideLimit(false) { }
	enum		SlideDirection { Stop, Out, In };
	void		Setup();
	void		Run();
	bool		Command(String s);

protected:
	Metronome	Timer;

	ScaledStepper* Slide;			// stepper for Slide control
	bool		SlideLimit;			// slide limit switch is active/closed

	ScaledStepper* Pan;				// stepper for Pan control

	bool		CommandStepper(String s, ScaledStepper* stepper, const char* name);
};

extern Control control;

#endif
