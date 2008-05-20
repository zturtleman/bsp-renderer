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

// BaseApp.h -- the basis for the Windows application

#ifndef _BASEAPP_H
#define _BASEAPP_H
#include <windows.h>
#if defined(DEBUG) | defined(_DEBUG)
#include <crtdbg.h>
#endif
#include <d3d9.h>
#include <d3dx9.h>
#include <tchar.h>

#include "DInput.h"
#include "Renderer.h"
#include "Q3Map.h"
#include "misc.h"

class BaseApp 
{
public:
	static LRESULT CALLBACK WinProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	BaseApp(void);
	~BaseApp(void);	
	int run(void);

	void initDInput(void);
	void initRenderer(string cfgFile);

	LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void createDevice(void);
	void createWindow(void);
	void createFont(void);
	void draw(float dt);
	void onLostDevice(void);
	void onResetDevice(void);
	void switchFullScreen(void);	
	bool isDeviceLost(void);
	void updateScene(const float dt);	
	void updateFpsCounter(const float dt);	

	HWND mhWinHandle;	
	IDirect3D9* md3dInterface;
	IDirect3DDevice9* md3dDevice;
	D3DPRESENT_PARAMETERS md3dPP;
	ID3DXFont*          mFont;
	TCHAR fpsText[45];
	bool mFullScreen;
	bool mPaused;
	DInput *mDInput;
	Renderer *mRenderer;
	Q3Map *mQ3Map;
};

extern BaseApp *gBaseApp;
#endif /* _BASEAPP_H */