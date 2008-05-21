/*
===========================================================================
Copyright (C) 2008 Daniel Örstadius

This file is part of bsp-renderer source code.

bsp-renderer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

bsp-renderer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with bsp-renderer.  If not, see <http://www.gnu.org/licenses/>.

*/

// DInput.h -- interface to DirectInput

#ifndef _DINPUT_H
#define _DINPUT_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "misc.h"

class DInput
{
public:
	DInput(DWORD keyboardFlags, DWORD mouseFlags, HWND hwnd);
	~DInput();

	void poll(void);
	bool keyDown(const char key) const;  
	bool mouseButtonDown(const int button) const;
	float mouseDeltaX(void) const; 
	float mouseDeltaY(void) const;
	float mouseDeltaZ(void) const;

private:
	IDirectInput8*       mDInput;

	IDirectInputDevice8* mKeyboardDevice;
	char                 mKeyboardState[256]; 

	IDirectInputDevice8* mMouseDevice;
	DIMOUSESTATE2        mMouseState;
};

#endif /* _DINPUT_H */