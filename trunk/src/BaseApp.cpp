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

// BaseApp.cpp -- the basis for the Windows application

#include <windows.h>
#include <iostream>

#include "BaseApp.h"

#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 

using namespace std;

BaseApp *gBaseApp;

BaseApp::BaseApp(void)
{
  mFullScreen = false;
  mPaused = false;
  sprintf_s(mFpsText, sizeof(mFpsText), _T(""));	
  sprintf_s(mCollText, sizeof(mFpsText), _T(""));	
  mFontColor = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);

  createWindow();
  createDevice();
  createFont();		
}

BaseApp::~BaseApp(void)
{
  ReleaseCOM(mFont);
  ReleaseCOM(md3dDevice);
  ReleaseCOM(md3dInterface);		

  DELETE_P(mDInput);	
  DELETE_P(mRenderer);	
  DELETE_P(mQ3Map);	
}

LRESULT BaseApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT clientRect = {0, 0, 0, 0};
  
	switch (msg)
	{
    case WM_SETCURSOR:            
      SetCursor( NULL ); // Turn off the mouse cursor.                 
      return 0;

		case WM_ACTIVATE:
			if( LOWORD(wParam) == WA_INACTIVE )
				mPaused = true;
			else
				mPaused = false;
			return 0;

		case WM_SIZE:				
			if (md3dDevice)
			{
				md3dPP.BackBufferWidth  = LOWORD(lParam);
				md3dPP.BackBufferHeight = HIWORD(lParam);
				
				if( wParam == SIZE_MINIMIZED )
				{
					mPaused = true;					
				}
				else if( wParam == SIZE_MAXIMIZED )
				{
					mPaused = false;
					onLostDevice();
					V(md3dDevice->Reset(&md3dPP));
					onResetDevice();
				}
				else if( wParam == SIZE_RESTORED )
				{					
					mPaused = false;

					if( md3dPP.Windowed )
					{
						onLostDevice();
						V(md3dDevice->Reset(&md3dPP));
						onResetDevice();
					}
				}
			}
			break;

		case WM_EXITSIZEMOVE:			
			GetClientRect(mhWinHandle, &clientRect);
			md3dPP.BackBufferWidth  = clientRect.right;
			md3dPP.BackBufferHeight = clientRect.bottom;
			onLostDevice();
			V(md3dDevice->Reset(&md3dPP));
			onResetDevice();
			
			return 0;

		case WM_CLOSE:
			DestroyWindow(mhWinHandle);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:
			switch (wParam)
			{				
				case VK_ESCAPE:
					SendMessage( mhWinHandle, WM_CLOSE, 0, 0 );
					break;
        case VK_F1:
          mRenderer->setCollMode(R_COLL_MODE_DISABLED);
          sprintf_s(mCollText, sizeof(mCollText), _T("no collision detection"));
          break;
        case VK_F2:
          mRenderer->setCollMode(R_COLL_MODE_NO_GRAVITY);
          sprintf_s(mCollText, sizeof(mCollText), _T("collision detection, no gravity"));
          break;
        case VK_F3:
          mRenderer->setCollMode(R_COLL_MODE_GRAVITY);
          sprintf_s(mCollText, sizeof(mCollText), _T("collision detection, with gravity"));
          break;
        case VK_F8:			
					switchFullScreen();
					break;
			}			
	}

  return ::DefWindowProc(mhWinHandle, msg, wParam, lParam);
}

void BaseApp::createWindow(void)
{
  HINSTANCE hInstance = GetModuleHandle(0);	
  WNDCLASS winclass;

  winclass.lpszClassName = "bsp-renderer";
  winclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winclass.style         = CS_HREDRAW | CS_VREDRAW;
  winclass.lpfnWndProc   = WinProc; 
  winclass.cbClsExtra    = 0;
  winclass.cbWndExtra    = 0;
  winclass.hInstance     = hInstance;
  winclass.hIcon         = LoadIcon(0, IDI_APPLICATION);
  winclass.hCursor       = LoadCursor(0, IDC_ARROW);	
  winclass.lpszMenuName  = 0;

  if (!RegisterClass(&winclass))
  {
    return;
  }

  RECT rect = {0, 0, 800, 600};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

  mhWinHandle = CreateWindow(
    "bsp-renderer",
    "bsp-renderer, press F8 to toggle full screen",
    WS_OVERLAPPEDWINDOW,
    0,
    0,
    rect.right,
    rect.bottom,
    0,
    0,
    hInstance,
    0);

  if (mhWinHandle == 0)
  {
    cout << "window creation failed. exiting.\n";
    Sleep(3000);
    exit(0);
  }

  ShowWindow(mhWinHandle, SW_SHOW);
  UpdateWindow(mhWinHandle);
  SetWindowPos(mhWinHandle, HWND_TOP, 100, 100, rect.right, rect.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
}

int BaseApp::run(void)
{
  MSG  msg;

  ZeroMemory(&msg, sizeof(MSG));
  msg.message = WM_NULL;

  __int64 countsPerSec = 0;
  QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
  float secsPerCount = 1.0f / (float)countsPerSec;

  __int64 oldTimeStamp = 0;
  QueryPerformanceCounter((LARGE_INTEGER*)&oldTimeStamp);

  while(msg.message != WM_QUIT)
  {

    if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }	
    else
    {	
      if( mPaused )
      {
        Sleep(20);
        continue;
      }

      if( !isDeviceLost() )
      {
        __int64 timeStamp = 0;
        QueryPerformanceCounter((LARGE_INTEGER*)&timeStamp);
        float delta = (timeStamp - oldTimeStamp)*secsPerCount;

        updateScene(delta);
        draw(delta);

        oldTimeStamp = timeStamp;
      }			
    }
  }

  return (int)msg.wParam;	
}

void BaseApp::createDevice(void)
{
  md3dInterface = Direct3DCreate9(D3D_SDK_VERSION);	

  if (!md3dInterface)
  {
    return;
  }

  md3dPP.BackBufferFormat			      = D3DFMT_UNKNOWN;
  md3dPP.BackBufferWidth			      = 0;
  md3dPP.BackBufferHeight			      = 0;	
  md3dPP.BackBufferCount            = 1;
  md3dPP.MultiSampleType            = D3DMULTISAMPLE_NONE;
  md3dPP.MultiSampleQuality         = 0;
  md3dPP.SwapEffect                 = D3DSWAPEFFECT_DISCARD; 
  md3dPP.hDeviceWindow              = mhWinHandle;
  md3dPP.Windowed                   = true;
  md3dPP.EnableAutoDepthStencil     = true; 
  md3dPP.AutoDepthStencilFormat     = D3DFMT_D24S8;
  md3dPP.Flags                      = 0;
  md3dPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
  md3dPP.PresentationInterval       = D3DPRESENT_INTERVAL_DEFAULT;
  //md3dPP.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

  /*if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) &&
      (caps.DevCaps & D3DCREATE_PUREDEVICE))
  {
    flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
  }
  else
  {
    flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  }*/

  DWORD flags;
  D3DCAPS9 caps;
  V(md3dInterface->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps));
  
#ifdef NO_HW_VERTEXPROCESSING
  flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
#else
  flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
#endif

#ifndef NO_HW_VERTEXPROCESSING
  if (!(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) ||
      !(caps.DevCaps & D3DCREATE_PUREDEVICE))
  {
    cout << "NO_HW_VERTEXPROCESSING is not defined, and the program \n";
    cout << "is running on a computer without HW vertex processing.\n";
    cout << "Define NO_HW_VERTEXPROCESSING and recompile, or \n";
    cout << "run the appropriate executable.\n";
    cout << "Exiting in 20 seconds.\n";
    Sleep(20000);
    exit(0);
  }
#endif

  V(md3dInterface->CreateDevice(
    D3DADAPTER_DEFAULT, 
    D3DDEVTYPE_HAL,           
    mhWinHandle,          
    flags,
    &md3dPP,            
    &md3dDevice));
}

void BaseApp::createFont(void)
{
  TCHAR               strFont[32];	
  strcpy_s(strFont, 32, _T("Arial"));

  HDC hDC = GetDC( NULL );
  int nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
  ReleaseDC( NULL, hDC );

  int nHeight = -0 * nLogPixelsY / 72;
  V(D3DXCreateFont( md3dDevice,            
    nHeight,            
    0,                  
    FW_BOLD,            
    1,                  
    FALSE,              
    DEFAULT_CHARSET,    
    OUT_DEFAULT_PRECIS, 
    DEFAULT_QUALITY,    
    DEFAULT_PITCH | FF_DONTCARE, 
    strFont,              
    &mFont));             	
}

void BaseApp::updateScene(const float dt)
{		
  mDInput->poll();	

  mRenderer->update(dt);	
}

void BaseApp::draw(float dt)
{	
  // Clear the render target and the zbuffer 
  V( md3dDevice->Clear(0, NULL, /* D3DCLEAR_TARGET | */ D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 20, 20, 20), 1.0f, 0) );

  // Render the scene
  if( SUCCEEDED( md3dDevice->BeginScene() ) )
  {		
    mRenderer->draw();

    // draw the fps counter
    RECT rc, rc2, rc3;
    SetRect( &rc, 0, 0 , 50, 20 );    
    SetRect( &rc2, 0, 20 , 50, 40 );
    SetRect( &rc3, 0, 40 , 50, 60 );
    updateFpsCounter(dt);
    mFont->DrawText( NULL, "bsp-renderer", -1, &rc, DT_NOCLIP, mFontColor);
    mFont->DrawText( NULL, mCollText, -1, &rc2, DT_NOCLIP, mFontColor);
    mFont->DrawText( NULL, mFpsText, -1, &rc3, DT_NOCLIP, mFontColor);

    V(md3dDevice->EndScene());		

    // present the buffer
    V(md3dDevice->Present(0, 0, 0, 0));
  }
}

void BaseApp::onLostDevice(void)
{
  SUCCEEDED(mFont->OnLostDevice());
  mRenderer->onLostDevice();
}

void BaseApp::onResetDevice(void)
{
  SUCCEEDED(mFont->OnResetDevice());

  mRenderer->onResetDevice(&md3dPP);

  mRenderer->setupState();
}

void BaseApp::switchFullScreen(void)
{
  if (mFullScreen)
  {
    RECT rect = {0, 0, 800, 600};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    md3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
    md3dPP.BackBufferWidth  = 800;
    md3dPP.BackBufferHeight = 600;
    md3dPP.Windowed         = true;		

    SetWindowLongPtr(mhWinHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);

    SetWindowPos(mhWinHandle, HWND_TOP, 100, 100, rect.right, rect.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);

    mFullScreen = false;
  }
  else
  {		
    int width  = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    md3dPP.BackBufferFormat = D3DFMT_A8R8G8B8;
    md3dPP.BackBufferWidth  = width;
    md3dPP.BackBufferHeight = height;
    md3dPP.Windowed         = false;

    SetWindowLongPtr(mhWinHandle, GWL_STYLE, WS_POPUP);

    SetWindowPos(mhWinHandle, HWND_TOP, 0, 0, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);	

    mFullScreen = true;    
  }

  onLostDevice();
  V(md3dDevice->Reset(&md3dPP));
  onResetDevice();
}

bool BaseApp::isDeviceLost()
{	
  HRESULT hr = md3dDevice->TestCooperativeLevel();

  if( hr == D3DERR_DEVICELOST )
  {
    Sleep(20);
    return true;
  }
  else if( hr == D3DERR_DRIVERINTERNALERROR )
  {		
    PostQuitMessage(0);
    return true;
  }	
  else if( hr == D3DERR_DEVICENOTRESET )
  {
    onLostDevice();
    V(md3dDevice->Reset(&md3dPP));
    onResetDevice();
    return false;
  }
  else
  {
    return false;
  }
}

void BaseApp::initDInput(void)
{
  mDInput = new DInput(DISCL_NONEXCLUSIVE|DISCL_BACKGROUND, DISCL_NONEXCLUSIVE|DISCL_BACKGROUND, mhWinHandle);

  if (mDInput == NULL)
  {
    cout << "failed to create direct input object. exiting.\n";
    SendMessage( mhWinHandle, WM_CLOSE, 0, 0 );
  }
}

void BaseApp::initRenderer(string cfgFile)
{
  mRenderer = new Renderer();
  mQ3Map = new Q3Map();

  mQ3Map->loadMap(cfgFile);

  mRenderer->setMap(mQ3Map);
  mRenderer->setDXDevice(md3dDevice);
  mRenderer->initRenderer();
  mRenderer->setDInput(mDInput);
  mRenderer->createSkyFX();
  mRenderer->setCollMode(R_COLL_MODE_DISABLED);

  sprintf_s(mCollText, sizeof(mCollText), _T("no collision detection"));

  onResetDevice();	
}

void BaseApp::updateFpsCounter(const float dt)
{	
  static float frameCounter   = 0.0f;
  static float timeCounter    = 0.0f;
  float framespersecond;

  frameCounter += 1.0f;	
  timeCounter  += dt;

  if( timeCounter >= 1.0f )
  {
    framespersecond = frameCounter / timeCounter;

    timeCounter    = 0.0f;
    frameCounter   = 0.0f;

    sprintf_s(mFpsText, sizeof(mFpsText), _T("fps: %.2f"), framespersecond);
  }	
}

LRESULT CALLBACK BaseApp::WinProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (gBaseApp != NULL)
  {
    return gBaseApp->msgProc(msg, wParam, lParam);
  }
  else
  {
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
  }
}