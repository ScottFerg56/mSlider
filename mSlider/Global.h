/*

  @@@@    @@@           @@@               @@@
 @@  @@    @@            @@                @@
@@    @    @@            @@                @@
@@         @@    @@@@@   @@@@    @@@@      @@
@@         @@   @@   @@  @@ @@      @@     @@
@@ @@@@    @@   @@   @@  @@  @@  @@@@@     @@
@@   @@    @@   @@   @@  @@  @@ @@  @@     @@
 @@  @@    @@   @@   @@  @@  @@ @@  @@     @@
  @@@ @   @@@@   @@@@@   @@@@@   @@@ @@   @@@@
  
 Author:	Scott Ferguson
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
