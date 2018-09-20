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

 Author:	Scott Ferguson
*/

#ifndef Control_h
#define Control_h

#include "mSlider.h"
#include "ScaledStepper.h"

/// <summary>An Applet for controlling the slider/panner motors and camera shutter.</summary>
/// <remarks>
/// Control receives Command input (e.g. from the BlueCtrl and debug Applets) to control the stepper motors
/// for Slide and Pan functions as well as the camera shutter interface for use as an intervalometer.
/// </remarks>
class Control : public Applet
{
public:
	Control() : Timer(500) { }
	void		Setup();
	void		Run();
	bool		Command(String s);

protected:
	/// <summary>The state of a shutter trigger sequence.</summary>
	enum ShutterStatus
	{
		Idle,		// no trigger sequence active
		Init,		// shutter and focus controls initialized
		Focus,		// focus control triggered
		Shutter,	// shutter control triggered
		Done		// controls released
	};

	/// <summary>Stepper properties exposed to the Command interface.</summary>
	/// <remarks>The enum values represent the character codes used in the Command strings.</remarks>
	enum Properties { Prop_Position = 'p', Prop_Acceleration = 'a', Prop_Speed = 's', Prop_MaxSpeed = 'm', Prop_SpeedLimit = 'l', Prop_Homed = 'h', Prop_TargetPosition = 't' };

	/// <summary>Camera properties exposed to the Command interface.</summary>
	/// <remarks>The enum values represent the character codes used in the Command strings.</remarks>
	enum CamProperties { Cam_FocusDelay = 'd', Cam_ShutterHold = 's', Cam_Interval = 'i', Cam_Frames = 'f' };

	Metronome	Timer;					// interval timer for feedback to controlling app via Bluetooth

	int			Action = 0;				// state recovery variable used by the controlling app

	ShutterStatus ShutterAction = Idle;	// next shutter action to take
	uint32_t	ShutterTime;			// in ms - time for next shutter action
	uint		FocusDelay = 150;		// in ms - delay after focus before tripping shutter
	uint		ShutterHold = 50;		// in ms - time to hold shutter signal
	uint		CamInterval = 0;		// in ms - time between camera frames
	uint		CamFrames = 0;			// # frames remaining to shoot

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
	void		SendProp(ScaledStepper* stepper, Properties prop, bool echo = false);
	void		SetCamProp(CamProperties prop, uint v);
	void		SendCamProp(CamProperties prop, bool echo = false);
};

#endif
