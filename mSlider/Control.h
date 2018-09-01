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
	Control() : Timer(500), SlideLimit(false), Homed(false), FocusDelay(500), ShutterAction(Idle),
		LastSlideStatus(ScaledStepper::Stopped), LastPanStatus(ScaledStepper::Stopped) { }
	enum		SlideDirection { Stop, Out, In };
	void		Setup();
	void		Run();
	bool		Command(String s);

protected:
	enum ShutterStatus { Idle, Init, Focus, Shutter, Done };
	enum Properties { Prop_Position = 'p', Prop_Acceleration = 'a', Prop_Speed = 's', Prop_MaxSpeed = 'm', Prop_SpeedLimit = 'l', Prop_Homed = 'h' };

	Metronome	Timer;

	ShutterStatus ShutterAction;	// next shutter action to take
	uint32_t	ShutterTime;		// in ms - time for next shutter action
	uint32_t	FocusDelay;			// in ms - delay after focus before tripping shutter

	ScaledStepper* Slide;			// stepper for Slide control
	ScaledStepper::RunStatus LastSlideStatus;
	float		LastSlideSpeed;
	bool		SlideLimit;			// slide limit switch is active/closed
	bool		Homed;				// slide limit has been reached at least once

	ScaledStepper* Pan;				// stepper for Pan control
	ScaledStepper::RunStatus LastPanStatus;
	float		LastPanSpeed;

	bool		CommandStepper(String s, ScaledStepper* stepper, const char* name);
	bool		CommandCamera(String s);
	void		SetProp(ScaledStepper* stepper, Properties prop, float v);
	void		SendProp(ScaledStepper* stepper, Properties prop);
};

extern Control control;

#endif
