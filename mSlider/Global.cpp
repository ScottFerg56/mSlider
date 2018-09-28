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

#include "Global.h"

/// <summary>Set a property value.</summary>
/// <param name="prop">The property to set.</param>
/// <param name="v">The value to set.</param>
bool Global::SetProp(char prop, const String& v)
{
	switch ((Properties)prop)
	{
	case Prop_Action:
		Action = v.toInt();
		break;
	default:
		return false;
	}
	return true;
}

/// <summary>Get a property value as a string.</summary>
/// <param name="prop">The property to get.</param>
String Global::GetProp(char prop)
{
	switch ((Properties)prop)
	{
	case Prop_Action:
		return String(Action);
	default:
		return (String)NULL;
	}
}
