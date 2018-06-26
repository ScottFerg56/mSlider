// 
// 
// 

#include "ScaledStepper.h"

ScaledStepper::RunStatus ScaledStepper::Run()
{
	if (!IsMoving)
	{
		return Stopped;
	}

	// don't let the stepper move beyond the set limits
	long dist = Stepper->distanceToGo();
	if (dist != 0)
	{
		if (Stepper->currentPosition() >= MaxLimit && dist > 0
			|| Stepper->currentPosition() <= MinLimit && dist < 0)
		{
			return ReachedGoal;
		}
	}

	if (!Stepper->run())
	{
		IsMoving = false;
		MoveStopTime = millis();
		if (Stepper->distanceToGo() == 0)
			return ReachedGoal;
		return Stopped;
	}
	return Moving;
}

void ScaledStepper::Stop()
{
	Stepper->stop();
//	Stepper->moveTo(Stepper->currentPosition());
}

void ScaledStepper::SetZero()
{
	Stepper->setCurrentPosition(0);
	Stepper->moveTo(0);
	IsMoving = false;
	MoveStopTime = millis();
}

void ScaledStepper::MoveTo(float position)
{
	long goal = (long)(position * StepsPerUnit);
	if (goal > MaxLimit)
		goal = MaxLimit;
	else if (goal < MinLimit)
		goal = MinLimit;
	Stepper->moveTo(goal);
	IsMoving = true;
	MoveStartTime = millis();
}

float ScaledStepper::GetCurrentPosition()
{
	return Stepper->currentPosition() / StepsPerUnit;
}

void ScaledStepper::SetCurrentPosition(float position)
{
	Stepper->setCurrentPosition((long)(position * StepsPerUnit));
}

float ScaledStepper::GetTargetPosition()
{
	return Stepper->targetPosition() / StepsPerUnit;
}

float ScaledStepper::GetAcceleration()
{
	return Acceleration / StepsPerUnit;
}

void ScaledStepper::SetAcceleration(float accel)
{
	Acceleration = accel * StepsPerUnit;
	Stepper->setAcceleration(Acceleration);
}

float ScaledStepper::GetSpeed()
{
	return Stepper->speed() / StepsPerUnit;
}

void ScaledStepper::SetSpeed(float speed)
{
	return Stepper->setSpeed(speed * StepsPerUnit);
}

float ScaledStepper::GetMaxSpeed()
{
	return Stepper->maxSpeed() / StepsPerUnit;
}

void ScaledStepper::SetMaxSpeed(float speed)
{
	speed = abs(speed);
	if (SpeedLimit != 0 && speed > SpeedLimit)
		speed = SpeedLimit;
	speed *= StepsPerUnit;
	Stepper->setMaxSpeed(speed);
}

float ScaledStepper::GetSpeedLimit()
{
	return SpeedLimit;
}

void ScaledStepper::SetSpeedLimit(float speed)
{
	SpeedLimit = abs(speed);
}

void ScaledStepper::SetLimits(float min, float max)
{
	MinLimit = min * StepsPerUnit;
	MaxLimit = max * StepsPerUnit;
}

float ScaledStepper::GetScale()
{
	return StepsPerUnit;
}

void ScaledStepper::SetScale(float scale)
{
	StepsPerUnit = scale;
}

uint32_t ScaledStepper::GetMicrosPerStep()
{
	return 1000000L / Stepper->maxSpeed();
}

void ScaledStepper::SetMicrosPerStep(uint32_t usPerStep)
{
	Stepper->setMaxSpeed(1000000.0 / usPerStep);
}

float ScaledStepper::GetLastMoveTime()
{
	return (MoveStopTime - MoveStartTime) / 1000.0;
}

float ScaledStepper::GetDistanceToGo()
{
	return Stepper->distanceToGo() / StepsPerUnit;
}

float ScaledStepper::MaxSpeedForDistanceAndTime(float distance, float seconds)
{
	//
	// Calculate the speed required to move a specified distance over a specified time
	// with the acceleration already specified.
	//
	// Initial and final velocities are assumed equal to zero and acceleration and deceleration
	// values are assumed equal, though this should be easy to generalize for other cases.
	//
	// The area under a graph of velocity versus time is the distance traveled.
	//
	// The graph of velocity while accelerating is an upward-sloping line
	// and the area under the graph is a right triangle with height 'S' (the speed to solve for)
	// and length 'Ta' (the time spent accelerating)
	//
	// The graph of velocity while decelerating at the same rate is a mirror of the
	// acceleration case. So the total distance traveled while accelerating and decelerating
	// is the sum of the area of the two triangles:
	//      Dad = (1/2 • S•Ta) • 2
	//         = S•Ta
	//
	// The graph of the constant velocity between acceleration and deceleration is a horizontal
	// line and the area under the graph is a rectangle with height 'S' and length 'Ts':
	//      Ds = S•Ts
	//
	// The total distance traveled is:
	//      D = Dad + Ds
	//        = S•Ta + S•Ts
	//
	// Knowing the acceleration 'A' we can calculate Ta as:
	//      Ta = S / A
	//
	// And expressing total time as 'T':
	//      Ts = T - 2•Ta
	//
	// With substitution:
	//      D = S • (S / A) + S • (T - 2 • (S / A) )
	//        = S^2 / A + S•T - 2 • S^2 / A
	//        = -S^2 / A + S•T
	//
	// or, in quadratic form:
	//      S^2 / A - S•T + D = 0
	//
	// Solving for S as quadratic roots:
	//      S = (T ± SQRT(T^2 - 4•D/A) ) / (2 / A)
	//
	// The value of the discriminant [ T^2 - 4•D/A ] determines the number of solutions:
	//      < 0  -- No solution: not enough time to reach the distance
	//              with the given acceleration
	//      == 0 -- One solution: Ts == 0 and all the time is spent
	//              accelerating and decelerating
	//      > 0  -- Two solutions:
	//              The larger value represents an invalid solution where Ts < 0
	//              The smaller value represents a valid solution where Ts > 0
	//
	// NOTE: Using the discriminant we can calculate the minimum amount of time
	// required to travel the specified distance:
	//      T = SQRT(4 • D / A)
	//

	// convert acceleration from steps to units
	distance = abs(distance);
	float accel = Acceleration / StepsPerUnit;
	// calculate the discriminant
	float disc = seconds * seconds - 4 * distance / accel;
	float speed;
	if (disc == 0.0)
	{
		// one solution - all accelerating and decelerating
		speed = seconds / 2.0 * accel;
	}
	else if (disc > 0.0)
	{
		// two solutions - use only the smaller, valid solution
		speed = (seconds - sqrt(disc)) / 2.0 * accel;
	}
	else
	{
		// no solution - not enough time to get there
		// as a consolation, determine the minimum time required
		// and calculate the speed we hit after accelerating

		// calculate the time that makes the discriminant zero
		seconds = sqrt(4.0 * distance / accel);
		// same as above, but the discriminant is zero
		// return a negative value so the caller knows we actually failed
		speed = - seconds / 2.0 * accel;	// one solution - all accelerating and decelerating
	}
	// note that this speed may exceed our SpeedLimit
	return speed;
}
