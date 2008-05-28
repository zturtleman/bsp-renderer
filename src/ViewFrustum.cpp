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

// ViewFrustum.cpp -- functions to cull against the view frustum

#include <d3d9.h>
#include <d3dx9.h>
#include "ViewFrustum.h"

void ViewFrustum::update(D3DXMATRIX *view, D3DXMATRIX *projection )
{
  D3DXMATRIX fov;	
  D3DXMatrixMultiply( &fov, view, projection );

  // the right plane.
  mPlanes[0].a = fov._14 - fov._11;
  mPlanes[0].b = fov._24 - fov._21;
  mPlanes[0].c = fov._34 - fov._31;
  mPlanes[0].d = fov._44 - fov._41;

  // the left plane.
  mPlanes[1].a = fov._14 + fov._11;
  mPlanes[1].b = fov._24 + fov._21;
  mPlanes[1].c = fov._34 + fov._31;
  mPlanes[1].d = fov._44 + fov._41;

  // the top plane.
  mPlanes[2].a = fov._14 - fov._12;
  mPlanes[2].b = fov._24 - fov._22;
  mPlanes[2].c = fov._34 - fov._32;
  mPlanes[2].d = fov._44 - fov._42;

  // the bottom plane.
  mPlanes[3].a = fov._14 + fov._12;
  mPlanes[3].b = fov._24 + fov._22;
  mPlanes[3].c = fov._34 + fov._32;
  mPlanes[3].d = fov._44 + fov._42;

  // the far plane.
  mPlanes[4].a = fov._14 - fov._13;
  mPlanes[4].b = fov._24 - fov._23;
  mPlanes[4].c = fov._34 - fov._33;
  mPlanes[4].d = fov._44 - fov._43;

  // the near plane
  mPlanes[5].a = fov._13;
  mPlanes[5].b = fov._23;
  mPlanes[5].c = fov._33;
  mPlanes[5].d = fov._43;

  // Normalize the planes.
  D3DXPlaneNormalize( &mPlanes[0], &mPlanes[0] );
  D3DXPlaneNormalize( &mPlanes[1], &mPlanes[1] );
  D3DXPlaneNormalize( &mPlanes[2], &mPlanes[2] );
  D3DXPlaneNormalize( &mPlanes[3], &mPlanes[3] );
  D3DXPlaneNormalize( &mPlanes[4], &mPlanes[4] );
  D3DXPlaneNormalize( &mPlanes[5], &mPlanes[5] );
}

bool ViewFrustum::checkIfBoxInside( int *min, int *max )
{
  for( char p = 0; p < 6; p++ )
  {
    D3DXVECTOR3 vec( (float)min[0], (float)min[1], (float)min[2] );
    if( D3DXPlaneDotCoord( &mPlanes[p], &vec ) >= 0.0f )
      continue;	

    vec = D3DXVECTOR3((float)max[0], (float)min[1], (float)min[2] );
    if( D3DXPlaneDotCoord( &mPlanes[p], &vec) >= 0.0f )
      continue;

    vec = D3DXVECTOR3( (float)min[0], (float)max[1], (float)min[2] );
    if( D3DXPlaneDotCoord( &mPlanes[p], &vec) >= 0.0f )
      continue;

    vec = D3DXVECTOR3( (float)max[0], (float)max[1], (float)min[2] );
    if( D3DXPlaneDotCoord( &mPlanes[p], &vec) >= 0.0f )
      continue;

    vec = D3DXVECTOR3( (float)min[0], (float)min[1], (float)max[2] );
    if( D3DXPlaneDotCoord( &mPlanes[p], &vec) >= 0.0f )
      continue;

    vec = D3DXVECTOR3( (float)max[0], (float)min[1], (float)max[2] );
    if( D3DXPlaneDotCoord( &mPlanes[p], &vec) >= 0.0f )
      continue;

    vec = D3DXVECTOR3( (float)min[0], (float)max[1], (float)max[2] );
    if( D3DXPlaneDotCoord( &mPlanes[p], &vec) >= 0.0f )
      continue;

    vec = D3DXVECTOR3( (float)max[0], (float)max[1], (float)max[2] );
    if( D3DXPlaneDotCoord( &mPlanes[p], &vec) >= 0.0f )
      continue;

    return false;
  }

  return true;
}

