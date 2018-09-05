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

void Control::Setup()
{
	pinMode(SlideLimitPin, INPUT_PULLUP);
	pinMode(FocusPin, INPUT);
	pinMode(ShutterPin, INPUT);
	Slide = new ScaledStepper(new AccelStepper(AccelStepper::DRIVER, SlideStepPin, SlideDirPin), 80.0);
	Slide->Stepper->setPinsInverted(true);
	Slide->SetSpeedLimit(50);
	Slide->SetMaxSpeed(30);
//	Slide->Stepper->setMinStepInterval(250);
	Slide->SetAcceleration(25);

	Pan = new ScaledStepper(new AccelStepper(AccelStepper::DRIVER, PanStepPin, PanDirPin), 400.0 / 9);
	Pan->SetSpeedLimit(90);
	Pan->SetMaxSpeed(55);
//	Pan->Stepper->setMinStepInterval(250);
	Pan->SetAcceleration(45);

	// start moving toward limit switch to initialize home position
	Slide->MoveTo(-700);
}

void Control::Run()
{
	bool lim = digitalRead(SlideLimitPin) == LOW;
	if (SlideLimit != lim)
	{
		SlideLimit = lim;
		if (SlideLimit && Slide->GetDistanceToGo() < 0.0)
		{
			Slide->SetZero();
			SendProp(Slide, Prop_Position);
			Slide->SetLimits(0, 640);
			if (!Homed)
			{
				Homed = true;
				SendProp(Slide, Prop_Homed);
			}
			debug.println("Slide Hit Limit: ", Slide->GetCurrentPosition());
			debug.println("..secs: ", Slide->GetLastMoveTime());
		}
	}

	ScaledStepper::RunStatus status = Slide->Run();
	if (status != LastSlideStatus)
	{
		LastSlideStatus = status;
		if (status == ScaledStepper::ReachedGoal)
		{
			debug.println("Slide Reached Goal: ", Slide->GetCurrentPosition());
			debug.println("..secs: ", Slide->GetLastMoveTime());
			// Bluetooth send slide position
			SendProp(Slide, Prop_Position);
		}
	}

	status = Pan->Run();
	if (status != LastPanStatus)
	{
		LastPanStatus = status;
		if (status == ScaledStepper::ReachedGoal)
		{
			debug.println("Pan Reached Goal: ", Pan->GetCurrentPosition());
			debug.println("..secs: ", Pan->GetLastMoveTime());
			// Bluetooth send pan position
			SendProp(Pan, Prop_Position);
		}
	}

	if (Timer)
	{
		if (Slide->GetDistanceToGo() != 0)
		{
			// Bluetooth send slide position
			SendProp(Slide, Prop_Position);
		}
		if (Pan->GetDistanceToGo() != 0)
		{
			// Bluetooth send pan position
			SendProp(Pan, Prop_Position);
		}

		float speed = Slide->GetSpeed();
		if (speed != LastSlideSpeed)
		{
			LastSlideSpeed = speed;
			// Bluetooth send slide speed
			SendProp(Slide, Prop_Speed);
		}

		speed = Pan->GetSpeed();
		if (speed != LastPanSpeed)
		{
			LastPanSpeed = speed;
			// Bluetooth send pan speed
			SendProp(Pan, Prop_Speed);
		}
	}

	switch (ShutterAction)
	{
	case Control::Idle:
		break;
	case Control::Init:
		{
			uint32_t ms = millis();
			// set focus and shutter pins as outputs and delay for them to set up
			debug.println("Camera Init: ", ms);
			pinMode(FocusPin, OUTPUT);
			pinMode(ShutterPin, OUTPUT);
			digitalWrite(FocusPin, HIGH);
			digitalWrite(ShutterPin, HIGH);
			ShutterTime = ms + 20;
			ShutterAction = Focus;	// next action
		}
		break;
	case Control::Focus:
		{
			uint32_t ms = millis();
			if (ms >= ShutterTime)
			{
				debug.println("Camera Focus: ", ms);
				// ground focus pin to activate and delay for camera to do the focus
				digitalWrite(FocusPin, 0);
				ShutterTime = ms + FocusDelay;
				ShutterAction = Shutter;	// next action
			}
		}
		break;
	case Control::Shutter:
		{
			uint32_t ms = millis();
			if (ms >= ShutterTime)
			{
				debug.println("Camera Shutter: ", ms);
				// ground shutter pin to activate and delay for camera action
				digitalWrite(ShutterPin, 0);
				ShutterTime = ms + 50;
				ShutterAction = Done;	// next action
			}
		}
		break;
	case Control::Done:
		{
			uint32_t ms = millis();
			if (ms >= ShutterTime)
			{
				debug.println("Camera Done: ", ms);
				pinMode(FocusPin, INPUT);
				pinMode(ShutterPin, INPUT);
				ShutterAction = Idle;	// next action
			}
		}
		break;
	default:
		break;
	}
}

bool Control::Command(String s)
{
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
							Parent->Command("bsga", (uint)Action);
						}
						else
						{
							Action = s.substring(2).toInt();
							debug.println("Action set: ", Action);
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
			return false;
	}
	return true;
}

bool Control::CommandStepper(String s, ScaledStepper* stepper, const char* name)
{
	// s[0] == 's' for Slide; 'p' for Pan
	// s[1] == Property or Action
	// s[2] == '?' to get property or start of string value to set property

	if (s.length() < 3)
		return true;

	debug.println("Stepper: ", s);

	if (s[2] == '?')
	{
		SendProp(stepper, (Properties)s[1]);
		return true;
	}

	switch (s[1])
	{
		case 'v':	// Velocity -- Set the speed, with direction + or -
		{
			float speed = s.substring(2).toFloat();
			if (speed == 0)
			{
				stepper->Stop();
				break;
			}
			float goal = speed > 0 ? 99999 : -99999;
			if (speed < 0)
				speed = -speed;
			stepper->SetMaxSpeed(speed * stepper->GetSpeedLimit() / 100);
			stepper->MoveTo(goal);
		}
		break;

		case 't':	// Timing -- Set the microseconds per step
		{
			int us = s.substring(2).toInt();
			stepper->SetMicrosPerStep(us);
		}
		break;

		case 'w':	// Waypoint -- Move to a distance over a duration
		{
			// Move a specified distance (+/-) in a number of seconds, given two
			// comma-separated values.
			int i = s.indexOf(',');
			if (i >= 0)
			{
				float distance = s.substring(2, i).toFloat();
				float seconds = s.substring(i+1).toFloat();
				float speed = stepper->MaxSpeedForDistanceAndTime(distance, seconds);
				debug.println("->distance: ", distance);
				debug.println("->seconds: ", seconds);
				debug.println("->speed: ", speed);
				stepper->SetMaxSpeed(speed);
				stepper->MoveTo(stepper->GetCurrentPosition() + distance);
			}
		}
		break;

		case 'z':	// Zero -- 
			if (s[0] == 'p')	// only valid for pan
			{
				stepper->SetZero();
				SendProp(stepper, Prop_Position);
			}
			break;

		default:
			SetProp(stepper, (Properties)s[1], s.substring(2).toFloat());
			break;
	}
	return true;
}

bool Control::CommandCamera(String s)
{
	// s[0] == 'c' for Camera
	// s[1] == Property or Action
	// s[2] == '?' to get property or start of string value to set property

	if (s.length() < 3)
		return true;

	debug.println("Camera: ", s);

	if (s[2] == '?')
	{
		SendCamProp((CamProperties)s[1]);
		return true;
	}

	switch ((CamProperties)s[1])
	{
	default:
		SetCamProp((CamProperties)s[1], s.substring(2).toFloat());
		break;
	}
	return true;
}

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
	//	stepper->SetSpeed(v);	// no need to ever actually set Speed
		break;
	case Prop_MaxSpeed:
		stepper->SetMaxSpeed(v);
		break;
	case Prop_SpeedLimit:
		stepper->SetSpeedLimit(v);
		break;
	case Prop_Homed:
		if (v == 0 && stepper == Slide)
		{
			Homed = false;
			SendProp(stepper, Prop_Homed);
			// start moving toward limit switch to initialize home position
			stepper->MoveTo(-700);
		}
		break;
	default:
		break;
	}
}

void Control::SendProp(ScaledStepper* stepper, Properties prop)
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
	default:
		break;
	}
	Parent->Command(String("bs") + prefix + (char)prop, v);
}

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
		break;
	default:
		break;
	}
}

void Control::SendCamProp(CamProperties prop)
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
	Parent->Command(String("bs") + prefix + (char)prop, v);
}
