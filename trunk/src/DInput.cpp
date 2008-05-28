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

// DInput.cpp -- interface to DirectInput

#include "DInput.h"
#include "misc.h"

DInput::DInput(DWORD keyboardFlags, DWORD mouseFlags, HWND hwnd)
{	
  HINSTANCE hInstance = GetModuleHandle(0);	

  ZeroMemory(mKeyboardState, sizeof(mKeyboardState));
  ZeroMemory(&mMouseState, sizeof(mMouseState));

  V(DirectInput8Create(
    hInstance, DIRECTINPUT_VERSION, 
    IID_IDirectInput8, (void**)&mDInput, NULL));

  HRESULT hr;

  hr = mDInput->CreateDevice(GUID_SysKeyboard, &mKeyboardDevice, 0);
  if (FAILED(hr))
  {
    //DInput_Exit();
    return;
  }

  V(mKeyboardDevice->SetDataFormat(&c_dfDIKeyboard));
  V(mKeyboardDevice->SetCooperativeLevel(hwnd, keyboardFlags));
  V(mKeyboardDevice->Acquire());

  V(mDInput->CreateDevice(GUID_SysMouse, &mMouseDevice, 0));
  V(mMouseDevice->SetDataFormat(&c_dfDIMouse2));
  V(mMouseDevice->SetCooperativeLevel(hwnd, mouseFlags));
  V(mMouseDevice->Acquire());
}

DInput::~DInput()
{
  mKeyboardDevice->Unacquire();
  mMouseDevice->Unacquire();
  ReleaseCOM(mKeyboardDevice);
  ReleaseCOM(mMouseDevice);
}

void DInput::poll(void)
{	
  HRESULT hr = mKeyboardDevice->GetDeviceState(sizeof(mKeyboardState), (void**)&mKeyboardState); 

  if( FAILED(hr) )
  {
    if (hr == DIERR_INPUTLOST)
    {
      // Keyboard lost
      ZeroMemory(mKeyboardState, sizeof(mKeyboardState));

      hr = mKeyboardDevice->Acquire();   
    }
  }

  hr = mMouseDevice->GetDeviceState(sizeof(DIMOUSESTATE2), (void**)&mMouseState); 

  if( FAILED(hr) )
  {
    if (hr == DIERR_INPUTLOST)
    {
      // Mouse lost
      ZeroMemory(&mMouseState, sizeof(mMouseState));

      hr = mMouseDevice->Acquire(); 
    }
  }
}

bool DInput::keyDown(const char key) const 
{
  return (mKeyboardState[key] & 0x80) != 0;
}

bool DInput::mouseButtonDown(const int button) const 
{
  return (mMouseState.rgbButtons[button] & 0x80) != 0;
}
float DInput::mouseDeltaX() const 
{
  return (float)mMouseState.lX;
}

float DInput::mouseDeltaY() const 
{
  return (float)mMouseState.lY;
}

float DInput::mouseDeltaZ() const 
{
  return (float)mMouseState.lZ;
}