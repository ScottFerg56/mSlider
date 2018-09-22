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

#include "Control.h"

/*
	Slide:
		+direction away from stepper
		red		-> A1
		green	-> A2
		yellow	-> B1
		blue	-> B2
		us/step min: 200
		3200 steps/rev  /  40 mm/rev == 80 steps/mm
		us/step = 1000000 / (speed mm/sec * 80 steps/mm)

	Pan:
		+direction clockwise
		red		-> A1
		green	-> A2
		yellow	-> B1
		blue	-> B2
		us/step min: 
		80T / 16T = 5:1
		With the stepper at 3200 steps/rev, the final stage is:
		16000 steps/rev  /  360 deg/rev == 400/9 steps/degree
*/

/// <summary>One-time Setup initialization for the Applet.</summary>
void Control::Setup()
{
	// setup IO pins
	pinMode(SlideLimitPin, INPUT_PULLUP);
	pinMode(FocusPin, INPUT);
	pinMode(ShutterPin, INPUT);

	// setup Slider stepper
	Slide = new ScaledStepper(new AccelStepper(AccelStepper::DRIVER, SlideStepPin, SlideDirPin), 80.0);
	Slide->Stepper->setPinsInverted(true);
	Slide->SetSpeedLimit(50);
	Slide->SetMaxSpeed(30);
	Slide->SetAcceleration(25);

	// setup Pan stepper
	Pan = new ScaledStepper(new AccelStepper(AccelStepper::DRIVER, PanStepPin, PanDirPin), 400.0 / 9);
	Pan->SetSpeedLimit(90);
	Pan->SetMaxSpeed(55);
	Pan->SetAcceleration(45);

	// start moving toward limit switch to initialize home position
	Slide->MoveTo(-700);
}

/// <summary>Run from the Arduino loop() via App.Run() to poll for control activity.</summary>
/// <remarks>
/// Monitor Slider and Pan movement and camera shutter controls.
/// </remarks>
void Control::Run()
{
	// check for Slide hitting the limit switch
	bool lim = digitalRead(SlideLimitPin) == LOW;
	if (SlideLimit != lim)
	{
		// there's a change
		SlideLimit = lim;
		if (SlideLimit && Slide->GetDistanceToGo() < 0.0)
		{
			// hit limit while moving toward it
			// NOTE: the mechanical switch may bounce while moving away!
			Slide->SetZero();					// (re)calibrate home position
			SendProp(Slide, Prop_Position);		// notify the controller
			Slide->SetLimits(0, 640);			// can't set limits before homing
			if (!Homed)
			{
				Homed = true;
				SendProp(Slide, Prop_Homed);	// if we were actively homing, notify the controller
			}
		//	debug.println("Slide Hit Limit: ", Slide->GetCurrentPosition());
		//	debug.println("..secs: ", Slide->GetLastMoveTime());
		}
	}

	// nudge the Slide
	ScaledStepper::RunStatus status = Slide->Run();
	if (status != LastSlideStatus)
	{
		// status change
		LastSlideStatus = status;
		if (status == ScaledStepper::ReachedGoal)
		{
		//	debug.println("Slide Reached Goal: ", Slide->GetCurrentPosition());
		//	debug.println("..secs: ", Slide->GetLastMoveTime());
			SendProp(Slide, Prop_Position);		// notify the controller
		}
	}

	// nudge the Pan
	status = Pan->Run();
	if (status != LastPanStatus)
	{
		// status change
		LastPanStatus = status;
		if (status == ScaledStepper::ReachedGoal)
		{
		//	debug.println("Pan Reached Goal: ", Pan->GetCurrentPosition());
		//	debug.println("..secs: ", Pan->GetLastMoveTime());
			SendProp(Pan, Prop_Position);		// notify the controller
		}
	}

	if (Timer)
	{
		// check for interesting changes and notify the controller
		if (Slide->GetDistanceToGo() != 0)
		{
			SendProp(Slide, Prop_Position);
		}
		if (Pan->GetDistanceToGo() != 0)
		{
			SendProp(Pan, Prop_Position);
		}

		float speed = Slide->GetSpeed();
		if (speed != LastSlideSpeed)
		{
			LastSlideSpeed = speed;
			SendProp(Slide, Prop_Speed);
		}

		speed = Pan->GetSpeed();
		if (speed != LastPanSpeed)
		{
			LastPanSpeed = speed;
			SendProp(Pan, Prop_Speed);
		}
	}

	// operate the cammera shutter
	switch (ShutterAction)
	{
	case Control::Idle:		// nothing to do
		break;
	case Control::Init:		// initialize shutter control pins for use
		{
			uint32_t ms = millis();
			// set focus and shutter pins as outputs and delay for them to set up
		//	debug.println("Camera Init: ", ms);
			pinMode(FocusPin, OUTPUT);
			digitalWrite(FocusPin, HIGH);
			pinMode(ShutterPin, OUTPUT);
			digitalWrite(ShutterPin, HIGH);
			ShutterTime = ms + 20;				// let the pins settle
			ShutterAction = Focus;				// next action
		}
		break;
	case Control::Focus:
		{
			uint32_t ms = millis();
			if (ms >= ShutterTime)				// wait
			{
			//	debug.println("Camera Focus: ", ms);
				digitalWrite(FocusPin, LOW);	// ground focus pin to activate
				ShutterTime = ms + FocusDelay;	// delay for camera to focus
				ShutterAction = Shutter;		// next action
			}
		}
		break;
	case Control::Shutter:
		{
			uint32_t ms = millis();
			if (ms >= ShutterTime)				// wait
			{
			//	debug.println("Camera Shutter: ", ms);
				digitalWrite(ShutterPin, LOW);	// ground shutter pin to activate
				ShutterTime = ms + ShutterHold;	// delay for camera action
				ShutterAction = Done;			// next action
			}
		}
		break;
	case Control::Done:
		{
			uint32_t ms = millis();
			if (ms >= ShutterTime)					// wait
			{
				if (CamFrames == 0 || --CamFrames == 0)
				{
					// number of frames complete
					SendCamProp(Cam_Frames);		// notify controller
				//	debug.println("Camera Done: ", ms);
					pinMode(FocusPin, INPUT);		// reset controls to inactive state
					pinMode(ShutterPin, INPUT);
					ShutterAction = Idle;			// next action
				}
				else
				{
					// one more fram complete
					SendCamProp(Cam_Frames);		// notify controller
				//	debug.println("Camera Frames: ", CamFrames);
					digitalWrite(FocusPin, HIGH);	// set controls to untriggered state
					digitalWrite(ShutterPin, HIGH);
					// delay for specified interval, or at least time for controls to settle
					if (CamInterval >= FocusDelay + ShutterHold)
						ShutterTime = ms + CamInterval - (FocusDelay + ShutterHold);
					else
						ShutterTime = ms + 20;
					ShutterAction = Focus;			// next action
				}
			}
		}
		break;
	default:
		break;
	}
}

/// <summary>Process a Command string, if recognized.</summary>
/// <param name="s">The Command string to be processed.</param>
/// <returns>True if recognized.</returns>
/// <remarks>
/// The Command string will be recognized with an initial character valid for related devices regardless of the contents of the remaining string.
/// Valid first characters:
///		'g' - global variables
///		'c' - camera
///		's' - Slide
///		'p' - Pan
/// The second character indicates a property to be accessed or an action.
/// The third character and beyond may be a value for the property (or action) or a '?' to retrieve the property value.
/// </remarks>
bool Control::Command(String s)
{
	// delegate based on the target device
	switch (s[0])
	{
		case 'g':	// GLOBAL
			{
				if (s.length() < 3)
					return true;

				switch (s[1])
				{
					case 'a':	// Action
					{
						// Action is a variable stored and retrieved by the control app
						// for recovery after disconnection
						// We don't care about the values
						if (s[2] == '?')
						{
							Parent->Output("ga" + String(Action));	// send requested value
						}
						else
						{
							Action = s.substring(2).toInt();
							debug.println("Action set: ", Action);	// set specified value
						}
						break;
					}
				}
			}
			break;

		case 'c':	// CAMERA
			return CommandCamera(s);

		case 's':	// SLIDE
			return CommandStepper(s, Slide, "Slide");

		case 'p':	// PAN
			return CommandStepper(s, Pan, "Pan");

		default:
			return false;	// not recognized
	}
	return true;	// recognized
}

/// <summary>Process a Command string for the Slide and Pan steppers.</summary>
/// <param name="s">The Command string to be processed.</param>
/// <returns>True.</returns>
/// <remarks>
/// The second character indicates a property to be accessed or an action. (See the Properties enum.)
/// The third character and beyond may be a value for the property (or action) or a '?' to retrieve the property value.
/// A double '?' causes the value to be dumped to the debug output as a debugging aid.
/// </remarks>
bool Control::CommandStepper(String s, ScaledStepper* stepper, const char* name)
{
	if (s.length() < 3)
		return true;

	debug.println("Stepper: ", s);

	if (s[2] == '?')
	{
		// send requested value to controller
		SendProp(stepper, (Properties)s[1], s.length() > 3 && s[3] == '?');
		return true;
	}

	switch (s[1])
	{
		case 'v':	// Velocity
		{
			// intended for manual movement control
			// the input value is a desired speed signed for direction
			float speed = s.substring(2).toFloat();
			if (speed == 0)
			{
				stepper->Stop();	// time to stop
				break;
			}
			// set an arbitrarily large goal to establish direction
			float goal = speed > 0 ? 99999 : -99999;
			if (speed < 0)
				speed = -speed;		// make it positive
			// speed is a percentage of the SpeedLimit, 0 to 100
			stepper->SetMaxSpeed(speed * stepper->GetSpeedLimit() / 100);
			SendProp(stepper, Prop_MaxSpeed);	// notify controller of MaxSpeed change
			stepper->MoveTo(goal);				// get moving
		}
		break;

		case 't':	// Timing
		{
			// set the microseconds per step
			// not used by the controller app, but useful for calibrating stepper performance
			int us = s.substring(2).toInt();
			stepper->SetMicrosPerStep(us);
		}
		break;

		case 'w':	// Waypoint Move
		{
			// Move a specified distance (+/-) in a number of seconds, given two comma-separated values.
			// not used by the controller app
			int i = s.indexOf(',');
			if (i >= 0)
			{
				// parse the parameters anc calculate required speed
				float distance = s.substring(2, i).toFloat();
				float seconds = s.substring(i+1).toFloat();
				float speed = stepper->MaxSpeedForDistanceAndTime(distance, seconds);
				debug.println("->distance: ", distance);
				debug.println("->seconds: ", seconds);
				debug.println("->speed: ", speed);
				// set required speed and start moving
				stepper->SetMaxSpeed(speed);
				stepper->MoveTo(stepper->GetCurrentPosition() + distance);
			}
		}
		break;

		case 'z':	// Zero
			if (s[0] == 'p')	// only valid for pan
			{
				stepper->SetZero();					// set current position as the new zero value
				SendProp(stepper, Prop_Position);	// notify the controller of Position change
			}
			break;

		default:
			// all others are properties to be set, value parsed from remainder of string
			SetProp(stepper, (Properties)s[1], s.substring(2).toFloat());
			break;
	}
	return true;
}

/// <summary>Process a Command string for the camera.</summary>
/// <param name="s">The Command string to be processed.</param>
/// <returns>True.</returns>
/// <remarks>
/// The second character indicates a property to be accessed or an action. (See the CamProperties enum.)
/// The third character and beyond may be a value for the property (or action) or a '?' to retrieve the property value.
/// A double '?' causes the value to be dumped to the debug output as a debugging aid.
/// </remarks>
bool Control::CommandCamera(String s)
{
	if (s.length() < 3)
		return true;

	debug.println("Camera: ", s);

	if (s[2] == '?')
	{
		// send requested value to controller
		SendCamProp((CamProperties)s[1], s.length() > 3 && s[3] == '?');
		return true;
	}

	switch ((CamProperties)s[1])
	{
	default:
		// all others are properties to be set, value parsed from remainder of string
		SetCamProp((CamProperties)s[1], s.substring(2).toFloat());
		break;
	}
	return true;
}

/// <summary>Set a property value for the Slide or Pan steppers.</summary>
/// <param name="stepper">The stepper being accessed.</param>
/// <param name="prop">The property being accessed.</param>
/// <param name="v">The value to set.</param>
void Control::SetProp(ScaledStepper* stepper, Properties prop, float v)
{
	switch (prop)
	{
	case Prop_Position:
		stepper->MoveTo(v);
		break;
	case Prop_Acceleration:
		stepper->SetAcceleration(v);
		break;
	case Prop_Speed:
		// no need to ever actually set Speed
		break;
	case Prop_MaxSpeed:
		stepper->SetMaxSpeed(v);
		break;
	case Prop_SpeedLimit:
		stepper->SetSpeedLimit(v);
		break;
	case Prop_Homed:
		if (v == 0 && stepper == Slide)		// only valid for Slide
		{
			Homed = false;
			SendProp(stepper, Prop_Homed);	// notify the controller of change
			stepper->SetLimits(-900, 900);	// relax limits for negative move
			stepper->MoveTo(-700);			// start moving toward limit switch
		}
		break;
	default:
		break;
	}
}

/// <summary>Send a property value for the Slide or Pan steppers to the controller app or debug output.</summary>
/// <param name="stepper">The stepper being accessed.</param>
/// <param name="prop">The property being accessed.</param>
/// <param name="echo">True to send to debug output, otherwise to controller.</param>
void Control::SendProp(ScaledStepper* stepper, Properties prop, bool echo)
{
	char prefix = stepper == Slide ? 's' : 'p';
	float v;
	switch (prop)
	{
	case Prop_Position:
		v = stepper->GetCurrentPosition();
		break;
	case Prop_Acceleration:
		v = stepper->GetAcceleration();
		break;
	case Prop_Speed:
		v = stepper->GetSpeed();
		break;
	case Prop_MaxSpeed:
		v = stepper->GetMaxSpeed();
		break;
	case Prop_SpeedLimit:
		v = stepper->GetSpeedLimit();
		break;
	case Prop_Homed:
		v = Homed ? 1 : 0;
		break;
	case Prop_TargetPosition:
		v = stepper->GetTargetPosition();
		break;
	default:
		break;
	}
	// format and output the value
	String s = String(prefix) + (char)prop;
	if (echo)
		debug.println(s.c_str(), v);
	else
		Parent->Output(s + String(v));
}

/// <summary>Set a property value for the camera.</summary>
/// <param name="prop">The property being accessed.</param>
/// <param name="v">The value to set.</param>
void Control::SetCamProp(CamProperties prop, uint v)
{
//	debug.println((String("--> ") + (char)prop).c_str(), v);
	switch (prop)
	{
	case Cam_FocusDelay:
		FocusDelay = v;
		break;
	case Cam_ShutterHold:
		ShutterHold = v;
		break;
	case Cam_Interval:
		CamInterval = v;
		break;
	case Cam_Frames:
		CamFrames = v;
		if (CamInterval > 0 && CamFrames > 0 && ShutterAction == Idle)
		{
			// setting the #frames starts intervalometer function
			ShutterAction = Init;
		}
		break;
	default:
		break;
	}
}

/// <summary>Send a property value for the camera to the controller app or debug output.</summary>
/// <param name="prop">The property being accessed.</param>
/// <param name="echo">True to send to debug output, otherwise to controller.</param>
void Control::SendCamProp(CamProperties prop, bool echo)
{
	char prefix = 'c';
	uint v;
	switch (prop)
	{
	case Cam_FocusDelay:
		v = FocusDelay;
		break;
	case Cam_ShutterHold:
		v = ShutterHold;
		break;
	case Cam_Interval:
		v = CamInterval;
		break;
	case Cam_Frames:
		v = CamFrames;
		break;
	default:
		break;
	}
	// format and output the value
	String s = String(prefix) + (char)prop;
	if (echo)
		debug.println(s.c_str(), v);
	else
		Parent->Output(s + String(v));
}
