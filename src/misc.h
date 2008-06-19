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

// misc.h -- various definitions and functions

#ifndef _MISC_H
#define _MISC_H

#include <windows.h>
#include <string>
#include <iostream>
#include <tchar.h>
#include <dxerr9.h>
#include <dxerr.h>

using namespace std;

#define PARAMETER_NOT_USED(x) x=x
void exitFunction(string reason);

#define ReleaseCOM(x) { if(x){ x->Release();x = 0; } }
#define DELETE_P(x) if(x) delete x;
#define DELETE_ARRAY(x) if(x) delete[] x;

#ifdef _DEBUG
#ifndef V
#define V(x)													\
{																\
  HRESULT hr = x;												\
  if( FAILED(hr))												\
{															\
  cout << "\nan error occured\nerror code: "; \
  cout << DXGetErrorString(hr); \
  cout << "\nexiting..."; \
  Sleep(10000); \
  exit(0); \
}															\
}																
#endif
#else
#ifndef V
#define V(x)	x;
#endif
#endif

#ifdef _DEBUG
#define DEBUG_OUTPUT(s) (cout << s << "\n")
#else
#define DEBUG_OUTPUT(s)
#endif

#endif /* _MISC_H */
