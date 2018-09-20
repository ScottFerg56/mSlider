/*

 @@@@@                    @@@              @@@   @@@@@     @
@@   @@                    @@               @@  @@   @@   @@
@@   @@                    @@               @@  @@   @@   @@
 @@      @@@@@   @@@@      @@    @@@@@    @@@@   @@     @@@@@@   @@@@@  @@ @@@  @@ @@@   @@@@@  @@ @@@
  @@@   @@   @@     @@     @@   @@   @@  @@ @@    @@@     @@    @@   @@  @@  @@  @@  @@ @@   @@  @@  @@
	@@  @@       @@@@@     @@   @@@@@@@ @@  @@      @@    @@    @@@@@@@  @@  @@  @@  @@ @@@@@@@  @@  @@
@@   @@ @@      @@  @@     @@   @@      @@  @@  @@   @@   @@    @@       @@  @@  @@  @@ @@       @@
@@   @@ @@   @@ @@  @@     @@   @@   @@ @@  @@  @@   @@   @@ @@ @@   @@  @@@@@   @@@@@  @@   @@  @@
 @@@@@   @@@@@   @@@ @@   @@@@   @@@@@   @@@ @@  @@@@@     @@@   @@@@@   @@      @@      @@@@@  @@@@
																		 @@      @@
																		@@@@    @@@@

 Author:	Scott Ferguson
*/

#ifndef _SCALEDSTEPPER_h
#define _SCALEDSTEPPER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <AccelStepper.h>

/// <summary>An implementation layer on top of AccelStepper to provide scaled units.</summary>
/// <remarks>
/// ScaledStepper allows user-friendly logical units to be used with an AccelStepper interface.
/// See the AccelStepper documentation for additional usage information.
/// </remarks>
class ScaledStepper
{
public:
	/// <summary>Construct with an AccelStepper object and scaling factor.</summary>
	/// <param name="stepper">A pointer to an AccelStepper object controlling the stepper motor.</param>
	/// <param name="stepsPerUnit">A scaling factor specifying the number of stepper steps per logical unit.</param>
	ScaledStepper(AccelStepper *stepper, float stepsPerUnit)
	{
		Stepper = stepper;
		StepsPerUnit = stepsPerUnit;
		SpeedLimit = 0;
		MaxLimit = 2000000000L;	// arbitrarily large values for longs
		MinLimit = -2000000000L;
	}

	/// <summary>Status of stepper movement.</summary>
	enum RunStatus
	{
		Stopped,		// Stepper has stopped
		ReachedGoal,	// Stepper just reached goal (next status will be Stopped)
		Moving			// Stepper is still moving
	};

	RunStatus	Run();
	void		Stop();
	void		SetZero();
	void		MoveTo(float position);
	float		GetCurrentPosition();
	void		SetCurrentPosition(float position);
	float		GetTargetPosition();
	float		GetAcceleration();
	void		SetAcceleration(float accel);
	float		GetSpeed();
	float		GetMaxSpeed();
	void		SetMaxSpeed(float speed);
	void		SetSpeedLimit(float speed);
	float		GetSpeedLimit();
	void		SetLimits(float min, float max);
	float		GetScale();
	void		SetScale(float scale);
	uint32_t	GetMicrosPerStep();
	void		SetMicrosPerStep(uint32_t);
	float		GetLastMoveTime();
	float		GetDistanceToGo();
	float		MaxSpeedForDistanceAndTime(float distance, float seconds);

	AccelStepper *Stepper;		// the implementation actually performing stepper movement

protected:
	float		StepsPerUnit;	// scale factor in steps per unit
	float		SpeedLimit;		// in units
	long		MaxLimit;		// in steps
	long		MinLimit;		// in steps
	bool		IsMoving;		// record of whether we're trying to move the stepper or not
	uint32_t	MoveStartTime;	// record of the start time of the last move, in microseconds
	uint32_t	MoveStopTime;	// record of the stop time of the last move, in microseconds
	float		Acceleration;	// steps per second per second (not scaled units)
};

#endif
