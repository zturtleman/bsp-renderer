
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

// FpsCamera.h -- the camera class

#ifndef _CAMERA_H
#define _CAMERA_H

#include <d3dx9.h>
#include "DInput.h"
#include "ViewFrustum.h"
#include "Collision.h"

enum {COLL_MODE_DISABLED, COLL_MODE_NO_GRAVITY, COLL_MODE_FULL};

class FpsCamera
{
public:
	FpsCamera();

	void setLens(float fov, float aspect, float nearZ, float farZ, float screenHeight, float screenWidth);
	void update(const float dt);
	void setDInput(DInput *DInput);		
	void setViewFrustum(ViewFrustum *viewFrustum);
	void setCollision(Collision *collision);

  void setCollMode(int mode);

	const D3DXMATRIX& view() const;
	const D3DXMATRIX& projection() const;
	const D3DXMATRIX& viewProj(void) const;	
	const D3DXVECTOR3* position(void) const;	

private:
	void buildViewMatrix(void);

	// FpsCamera related matrices.
	D3DXMATRIX mViewMatrix;
	D3DXMATRIX mProjectionMatrix;
	D3DXMATRIX mViewProjMatrix;

	// Relative to world space.
	D3DXVECTOR3 mPosition;	
	D3DXVECTOR3 mRightVector;
	D3DXVECTOR3 mUpVector;
	D3DXVECTOR3 mLookVector;
		
	float mSpeed;
  D3DXVECTOR3 mDir;
  float mScreenHeight;
  float mScreenWidth;

  int mCollMode;

	DInput *mDInput;	

	ViewFrustum *mViewFrustum;

	Collision *mCollision;
};

#endif /* _CAMERA_H */