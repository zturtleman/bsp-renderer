/*
===========================================================================
Copyright (C) 2008 Daniel Írstadius

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

// adaptation.h -- intended to provide an interface to a graphics backend (DirectX or OpenGL)
// this interface is only preliminary at the moment

#ifndef _ADAPTATION_H
#define _ADAPTATION_H
#include <d3d9.h>
#include <d3dx9.h>

#define vec3f D3DXVECTOR3
#define vec3dot(x,y) D3DXVec3Dot(x,y) 

#endif /* _ADAPTATION_H */