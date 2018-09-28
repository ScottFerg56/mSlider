/*

  OOOO    OOO           OOO               OOO
 OO  OO    OO            OO                OO
OO    O    OO            OO                OO
OO         OO    OOOOO   OOOO    OOOO      OO
OO         OO   OO   OO  OO OO      OO     OO
OO OOOO    OO   OO   OO  OO  OO  OOOOO     OO
OO   OO    OO   OO   OO  OO  OO OO  OO     OO
 OO  OO    OO   OO   OO  OO  OO OO  OO     OO
  OOO O   OOOO   OOOOO   OOOOO   OOO OO   OOOO

    (c) 2018 Scott Ferguson
    This code is licensed under MIT license (see LICENSE file for details)
 */

#ifndef _Global_h
#define _Global_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <FMDebug.h>
#include <Applet.h>

///<summary>An Applet for Global properties exchanged with the Slider controller application.</summary>
class Global : public Applet
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="prefix">The character code to associate with this Applet.</param>
	Global(char prefix) : Applet(prefix) { Name = "Global"; }

	void		Setup() { }
	void		Run() { }
	String		GetProp(char prop);
	bool		SetProp(char prop, const String& v);

	/// <summary>Properties exposed to the communications interface.</summary>
	/// <remarks>The enum values represent the character codes used in the Input/Output strings.</remarks>
	enum Properties
	{
		Prop_Action = 'a',
	};

private:
	int			Action = 0;				// state recovery variable used by the controlling app
};

#endif
