// ScaledStepper.h

#ifndef _SCALEDSTEPPER_h
#define _SCALEDSTEPPER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <AccelStepper.h>

class ScaledStepper
{
public:
	ScaledStepper(AccelStepper *stepper, float stepsPerUnit)
	{
		Stepper = stepper;
		StepsPerUnit = stepsPerUnit;
		SpeedLimit = 0;
		MaxLimit = 2000000000L;	// arbitrarily large values for longs
		MinLimit = -2000000000L;
	}

	enum RunStatus { Stopped, ReachedGoal, Moving };

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
	void		SetSpeed(float speed);
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

	AccelStepper *Stepper;

protected:
	float		StepsPerUnit;	// scale factor in steps per unit
	float		SpeedLimit;		// in units
	long		MaxLimit;		// in steps
	long		MinLimit;		// in steps
	bool		IsMoving;
	uint32_t	MoveStartTime;	// in us
	uint32_t	MoveStopTime;	// in us
	float		Acceleration;	// steps per second per second (not scaled units)
};

#endif
