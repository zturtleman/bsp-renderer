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

// ViewFrustum.h -- functions to cull against the view frustum

#ifndef VIEW_FRUSTUM_H
#define VIEW_FRUSTUM_H

#include <d3d9.h>
#include <d3dx9.h>

class ViewFrustum
{
public:
	void update( D3DXMATRIX *fov, D3DXMATRIX *projection );
	void update( D3DXMATRIX *fov);

	bool checkIfBoxInside( int *mins, int *maxs );

private:	
	D3DXPLANE mPlanes[6]; // the planes of the view frustum
};

#endif