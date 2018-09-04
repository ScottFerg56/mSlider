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
	Control() : Timer(500) { }
	enum		SlideDirection { Stop, Out, In };
	void		Setup();
	void		Run();
	bool		Command(String s);

protected:
	enum ShutterStatus { Idle, Init, Focus, Shutter, Done };
	enum Properties { Prop_Position = 'p', Prop_Acceleration = 'a', Prop_Speed = 's', Prop_MaxSpeed = 'm', Prop_SpeedLimit = 'l', Prop_Homed = 'h' };

	Metronome	Timer;

	int			Action = 0;

	ShutterStatus ShutterAction = Idle;	// next shutter action to take
	uint32_t	ShutterTime;			// in ms - time for next shutter action
	uint32_t	FocusDelay = 500;		// in ms - delay after focus before tripping shutter

	ScaledStepper* Slide;				// stepper for Slide control
	ScaledStepper::RunStatus LastSlideStatus = ScaledStepper::Stopped;
	float		LastSlideSpeed = 0;
	bool		SlideLimit = false;		// slide limit switch is active/closed
	bool		Homed = false;			// slide limit has been reached at least once

	ScaledStepper* Pan;					// stepper for Pan control
	ScaledStepper::RunStatus LastPanStatus = ScaledStepper::Stopped;
	float		LastPanSpeed = 0;

	bool		CommandStepper(String s, ScaledStepper* stepper, const char* name);
	bool		CommandCamera(String s);
	void		SetProp(ScaledStepper* stepper, Properties prop, float v);
	void		SendProp(ScaledStepper* stepper, Properties prop);
};

#endif
