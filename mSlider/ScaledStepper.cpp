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

#include "ScaledStepper.h"

/// <summary>Moves the stepper as required toward any target position that may be set.</summary>
/// <returns>The updated state of stepper movement.</returns>
/// <remarks>
/// Based on the time interval since the last call, the stepper may be moved at most one step toward any set target position.
/// Run() should be called frequently enough to achieve the desired speed and acceleration.
/// The call to Run() that achieves the target position will return a status of ReachedGoal.
/// Subsequent calls, with no intervening movement directives, will return a status of Stopped.
/// </remarks>
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

	// move the stepper
	if (!Stepper->run())
	{
		// movement is done
		IsMoving = false;
		// record the stop time
		MoveStopTime = millis();
		// status depends on if we reached the target
		if (Stepper->distanceToGo() == 0)
			return ReachedGoal;
		return Stopped;
	}
	return Moving;
}

/// <summary>Stops the stepper as quickly as possible.</summary>
/// <remarks>
/// Sets a new target position that causes the stepper to stop as quickly as possible, using the current speed and acceleration parameters.
/// </remarks>
void ScaledStepper::Stop()
{
	Stepper->stop();
//	Stepper->moveTo(Stepper->currentPosition());
}

/// <summary>Sets the current position as the new zero value for the stepper.</summary>
/// <remarks>
/// Has the AccelStepper side effect of setting both the target position and the current position and stopping movement.
/// Should probably best be used only when stopped.
/// </remarks>
void ScaledStepper::SetZero()
{
	SetCurrentPosition(0);
}

/// <summary>Sets the stepper moving toward a new target position.</summary>
/// <param name="position">The new target position, in logical units.</param>
void ScaledStepper::MoveTo(float position)
{
	// scale and limit the new target
	long goal = (long)(position * StepsPerUnit);
	if (goal > MaxLimit)
		goal = MaxLimit;
	else if (goal < MinLimit)
		goal = MinLimit;
	// set it moving
	Stepper->moveTo(goal);
	IsMoving = true;
	MoveStartTime = millis();
}

/// <summary>Get the current position.</summary>
/// <returns>The current position, in logical units.</returns>
float ScaledStepper::GetCurrentPosition()
{
	return Stepper->currentPosition() / StepsPerUnit;
}

/// <summary>Set the current position.</summary>
/// <param name="position">The new current position, in logical units.</param>
/// <remarks>
/// Has the AccelStepper side effect of setting both the target position and the current position and stopping movement.
/// Should probably best be used only when stopped.
/// </remarks>
void ScaledStepper::SetCurrentPosition(float position)
{
	Stepper->setCurrentPosition((long)(position * StepsPerUnit));
	// this will stop a current movement in progress
	IsMoving = false;
	MoveStopTime = millis();
}

/// <summary>Get the target position.</summary>
/// <returns>The target position, in logical units.</returns>
float ScaledStepper::GetTargetPosition()
{
	return Stepper->targetPosition() / StepsPerUnit;
}

/// <summary>Get the acceleration setting to be used by moves.</summary>
/// <returns>The acceleration, in logical units.</returns>
float ScaledStepper::GetAcceleration()
{
	return Acceleration / StepsPerUnit;
}

/// <summary>Set the acceleration setting to be used by moves.</summary>
/// <param name="accel">The desired acceleration, in logical units.</param>
void ScaledStepper::SetAcceleration(float accel)
{
	// record the Acceleration value, since we can't recover it from the AccelStepper implementation
	Acceleration = accel * StepsPerUnit;
	Stepper->setAcceleration(Acceleration);
}

/// <summary>Get the speed of current movement.</summary>
/// <returns>The speed of current movement, in logical units.</returns>
float ScaledStepper::GetSpeed()
{
	return Stepper->speed() / StepsPerUnit;
}

/// <summary>Get the maximum speed used for movement.</summary>
/// <returns>The maximum speed, in logical units.</returns>
float ScaledStepper::GetMaxSpeed()
{
	return Stepper->maxSpeed() / StepsPerUnit;
}

/// <summary>Set the maximum speed used for movement.</summary>
/// <param name="speed">The desired maximum speed used for movement, in logical units.</param>
/// <remarks>
/// Step movement will accelerate to this speed.
/// </remarks>
void ScaledStepper::SetMaxSpeed(float speed)
{
	speed = abs(speed);
	if (SpeedLimit != 0 && speed > SpeedLimit)
		speed = SpeedLimit;
	speed *= StepsPerUnit;
	Stepper->setMaxSpeed(speed);
}

/// <summary>Get the upper limit for speeds allowed for movement.</summary>
/// <returns>The maximum value allowed for setting speeds used for movement, in logical units.</returns>
float ScaledStepper::GetSpeedLimit()
{
	return SpeedLimit;
}

/// <summary>Set the upper limit for speeds allowed for movement.</summary>
/// <param name="speed">The maximum value allowed for setting speeds used for movement, in logical units.</param>
void ScaledStepper::SetSpeedLimit(float speed)
{
	SpeedLimit = abs(speed);
}

/// <summary>Set the positional limits for movement.</summary>
/// <param name="min">The minimum positional value for movement, in logical units.</param>
/// <param name="max">The maximum positional value for movement, in logical units.</param>
void ScaledStepper::SetLimits(float min, float max)
{
	MinLimit = min * StepsPerUnit;
	MaxLimit = max * StepsPerUnit;
}

/// <summary>Get the scale factor in steps per unit.</summary>
/// <returns>The scale factor in steps per unit.</returns>
float ScaledStepper::GetScale()
{
	return StepsPerUnit;
}

/// <summary>Get the scale factor in steps per unit.</summary>
/// <param name="scale">The scale factor in steps per unit.</param>
void ScaledStepper::SetScale(float scale)
{
	StepsPerUnit = scale;
}

/// <summary>Get the minimum number of microseconds per step.</summary>
/// <returns>The microseconds per step.</returns>
uint32_t ScaledStepper::GetMicrosPerStep()
{
	return 1000000L / Stepper->maxSpeed();
}

/// <summary>Set the minimum number of microseconds per step.</summary>
/// <param name="usPerStep">The microseconds per step.</param>
/// <remarks>
/// This is an alternative way of controlling MaxSpeed, most useful
/// when monitoring or establishing the performance characteristics of a stepper.
/// </remarks>
void ScaledStepper::SetMicrosPerStep(uint32_t usPerStep)
{
	Stepper->setMaxSpeed(1000000.0 / usPerStep);
}

/// <summary>Get the duration of the last completed move.</summary>
/// <returns>The duration, in milliseconds, of the last completed move.</returns>
float ScaledStepper::GetLastMoveTime()
{
	return (MoveStopTime - MoveStartTime) / 1000.0;
}

/// <summary>Get the remaining distance to be traveled for the current move.</summary>
/// <returns>The distance remaining, in logical units.</returns>
float ScaledStepper::GetDistanceToGo()
{
	return Stepper->distanceToGo() / StepsPerUnit;
}

/// <summary>
/// Calculate the speed required to move a specified distance over a specified time
/// with the acceleration already specified.
/// </summary>
/// <param name="distance">The signed distance to move, in logical units.</param>
/// <param name="seconds">The duration, in seconds, desired for the move.</param>
/// <returns>The target cruising speed for the move, for use with SetMaxSpeed.</returns>
/// <remarks>
/// Initial and final velocities are assumed equal to zero and acceleration and deceleration
/// values are assumed equal (using the value from SetAcceleration).
/// </remarks>
float ScaledStepper::MaxSpeedForDistanceAndTime(float distance, float seconds)
{
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
