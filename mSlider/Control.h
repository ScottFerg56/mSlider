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
	Control() : Timer(500), SlideLimit(false), Homed(false), FocusDelay(500), ShutterAction(Idle) { }
	enum		SlideDirection { Stop, Out, In };
	void		Setup();
	void		Run();
	bool		Command(String s);

protected:
	enum ShutterStatus { Idle, Init, Focus, Shutter, Done };

	Metronome	Timer;

	ShutterStatus ShutterAction;	// next shutter action to take
	uint32_t	ShutterTime;		// in ms - time for next shutter action
	uint32_t	FocusDelay;			// in ms - delay after focus before tripping shutter

	ScaledStepper* Slide;			// stepper for Slide control
	bool		SlideLimit;			// slide limit switch is active/closed
	bool		Homed;				// slide limit has been reached at least once

	ScaledStepper* Pan;				// stepper for Pan control

	bool		CommandStepper(String s, ScaledStepper* stepper, const char* name);
	bool		CommandCamera(String s);
};

extern Control control;

#endif
