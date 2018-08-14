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

Control	control;

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
			Parent->Command("bssp", 0);
			Slide->SetLimits(0, 640);
			if (!Homed)
			{
				Homed = true;
				Parent->Command("bssh1");
			}
			debug.println("Slide Hit Limit: ", Slide->GetCurrentPosition());
			debug.println("..secs: ", Slide->GetLastMoveTime());
		}
	}

	ScaledStepper::RunStatus status = Slide->Run();
	if (status == ScaledStepper::ReachedGoal)
	{
		debug.println("Slide Reached Goal: ", Slide->GetCurrentPosition());
		debug.println("..secs: ", Slide->GetLastMoveTime());
		// Bluetooth send slide position
		Parent->Command("bssp", Slide->GetCurrentPosition());
	}

	status = Pan->Run();
	if (status == ScaledStepper::ReachedGoal)
	{
		debug.println("Pan Reached Goal: ", Pan->GetCurrentPosition());
		debug.println("..secs: ", Pan->GetLastMoveTime());
		// Bluetooth send pan position
		Parent->Command("bspp", Pan->GetCurrentPosition());
	}

	if (Timer)
	{
#if true
		if (Slide->GetDistanceToGo() != 0)
		{
		//	debug.println("Slide Position: ", Slide->GetCurrentPosition());
		//	debug.println("..speed: ", Slide->GetSpeed());
			// Bluetooth send slide position
			Parent->Command("bssp", Slide->GetCurrentPosition());
		}
		if (Pan->GetDistanceToGo() != 0)
		{
		//	debug.println("Pan Position: ", Pan->GetCurrentPosition());
		//	debug.println("..speed: ", Pan->GetSpeed());
			// Bluetooth send pan position
			Parent->Command("bspp", Pan->GetCurrentPosition());
		}
#endif
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
		case 'c':	// CAMERA
			return CommandCamera(s);

		case 's':	// SLIDE
			return CommandStepper(s, Slide, "Slide");

		case 'p':	// PAN
			return CommandStepper(s, Pan, "Pan");

		case '!':
			{
				if (s.length() != 4)
					return false;
				if (s[1] != 'b')
					return false;
				bool pressed = s[3] == '1';
				switch (s[2])
				{
				case '1':
					if (pressed) Pan->MoveTo(0); break;
				case '2':
					if (pressed) Slide->MoveTo(0); break;
				case '3':
				case '4':
					break;
				case '5':	// up
					if (pressed) Pan->MoveTo(180); else Pan->Stop(); break;
				case '6':	// down
					if (pressed) Pan->MoveTo(-180); else Pan->Stop(); break;
				case '7':	// left
					if (pressed) Slide->MoveTo(640); else Slide->Stop(); break;
				case '8':	// right
					if (pressed) Slide->MoveTo(0); else Slide->Stop(); break;
				default:
					return false;
				}
			}
			break;
		default:
			return false;
	}
	return true;
}

bool Control::CommandStepper(String s, ScaledStepper* stepper, const char* name)
{
	if (s.length() < 2)
		return true;

	debug.println("Control: ", s);
	switch (s[1])
	{
		case 'v':	// Velocity -- Set the speed, with direction + or -
		{
			if (s.length() >= 3)
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
		}
		break;

		case 's':	// maxSpeed -- Set the maximum speed for moves
		{
			if (s.length() >= 3)
			{
				float speed = s.substring(2).toFloat();
				stepper->SetMaxSpeed(speed);
			}
		}
		break;

		case 't':	// Timing -- Set the microseconds per step
		{
			if (s.length() >= 3)
			{
				int us = s.substring(2).toInt();
				stepper->SetMicrosPerStep(us);
			}
		}
		break;

		case 'a':	// Acceleration -- Set the acceleration used for moves
		{
			if (s.length() >= 3)
			{
				float accel = s.substring(2).toFloat();
				stepper->SetAcceleration(accel);
			}
		}
		break;

		case 'w':	// timed move -- To a distance over a duration
		{
			// Move a specified distance (+/-) in a number of seconds, given two
			// comma-separated values.
			if (s.length() >= 3)
			{
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
		}
		break;

		case 'p':	// Position -- Move to a position
		{
			if (s.length() >= 3)
			{
				if (s[2] == '?')
				{
					// respond to query for the position
					// Bluetooth send slide/pan position
					Parent->Command(String("bs") + s[0] + 'p', stepper->GetCurrentPosition());
				}
				else
				{
					int position = s.substring(2).toFloat();
					stepper->MoveTo(position);
				}
			}
		}
		break;

		case 'h':	// Home - Get Homed state or request homing operation
			if (s[0] == 's')	// only valid for slide
			{
				if (s.length() >= 3 && s[2] == '?')
				{
					Parent->Command(Homed ? "bssh1" : "bssh0");
				}
				else
				{
					Homed = false;
					Parent->Command("bssh0");
					// start moving toward limit switch to initialize home position
					stepper->MoveTo(-700);
				}
			}
			break;

		case 'z':	// Zero -- 
			if (s[0] == 'p')	// only valid for pan
			{
				stepper->SetZero();
				Parent->Command("bspp", 0);
				debug.println(name, ": Set Zero");
				return true;
			}
			break;

		default:	// Position -- Shortcut
		{
			// Shortcut for 'Move to a position' not requiring the 'p' property qualifier
			int position = s.substring(1).toFloat();
			stepper->MoveTo(position);
		}
		break;
	}
	debug.println(name, ":");
	debug.println("..Target: ", stepper->GetTargetPosition());
	debug.println("..Max Speed: ", stepper->GetMaxSpeed());
	debug.println("..Speed Limit: ", stepper->GetSpeedLimit());
	debug.println("..usPerStep: ", stepper->GetMicrosPerStep());
	debug.println("..Acceleration: ", stepper->GetAcceleration());
	return true;
}

bool Control::CommandCamera(String s)
{
	if (s.length() < 2)
		return true;

	debug.println("Camera: ", s);
	switch (s[1])
	{
		case 's':	// Shutter -- trip the shutter
			ShutterAction = Init;
		break;

		case 'd':	// Delay -- Set the delay time in ms between focus and shutter release
		{
			if (s.length() >= 3)
			{
				FocusDelay = s.substring(2).toInt();
				debug.println("Focus delay: ", FocusDelay);
			}
		}
		break;
	}
	return true;
}
