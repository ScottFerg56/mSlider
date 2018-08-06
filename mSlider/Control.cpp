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
			Slide->SetLimits(0, 640);
			debug.println("Slide Hit Limit: ", Slide->GetCurrentPosition());
			debug.println("..secs: ", Slide->GetLastMoveTime());
		}
	}

	ScaledStepper::RunStatus status = Slide->Run();
	if (status == ScaledStepper::ReachedGoal)
	{
		debug.println("Slide Reached Goal: ", Slide->GetCurrentPosition());
		debug.println("..secs: ", Slide->GetLastMoveTime());
	}

	status = Pan->Run();
	if (status == ScaledStepper::ReachedGoal)
	{
		debug.println("Pan Reached Goal: ", Pan->GetCurrentPosition());
		debug.println("..secs: ", Pan->GetLastMoveTime());
	}

	if (Timer)
	{
#if false
		if (Slide->GetDistanceToGo() != 0)
		{
			debug.println("Slide Position: ", Slide->GetCurrentPosition());
			debug.println("..speed: ", Slide->GetSpeed());
		}
		if (Pan->GetDistanceToGo() != 0)
		{
			debug.println("Pan Position: ", Pan->GetCurrentPosition());
			debug.println("..speed: ", Pan->GetSpeed());
		}
#endif
	}
}

bool Control::Command(String s)
{
	debug.println("Command: ", s);
	switch (s[0])
	{
		case 's':
			return CommandStepper(s, Slide, "Slide");

		case 'p':
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
	{
		stepper->SetZero();
		debug.println(name, ": Set Zero");
		return true;
	}

	switch (s[1])
	{
		case 'v':
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

		case 's':
		{
			if (s.length() >= 3)
			{
				float speed = s.substring(2).toFloat();
				stepper->SetMaxSpeed(speed);
			}
		}
		break;

		case 't':
		{
			if (s.length() >= 3)
			{
				int us = s.substring(2).toInt();
				stepper->SetMicrosPerStep(us);
			}
		}
		break;

		case 'a':
		{
			if (s.length() >= 3)
			{
				float accel = s.substring(2).toFloat();
				stepper->SetAcceleration(accel);
			}
		}
		break;

		case 'w':
		{
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

		default:
		{
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
