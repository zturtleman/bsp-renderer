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
// main.cpp -- start up the engine

#include "Engine.h"
#include <iostream>

using namespace std;

//int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE prevInstance,
//				    PSTR cmdLine, int showCmd)
int main(void)
{			
	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

	Engine e;
	gEngine = &e;	

	gEngine->initDInput();
	gEngine->initRenderer("../media/bsp.cfg");

	gEngine->run();

	return 0;
}