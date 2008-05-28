
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

// FpsCamera.cpp -- the camera class

#include "FpsCamera.h"
#include "ViewFrustum.h"
#include "DInput.h"

FpsCamera::FpsCamera()
{
  D3DXMatrixIdentity(&mViewMatrix);
  D3DXMatrixIdentity(&mProjectionMatrix);
  D3DXMatrixIdentity(&mViewProjMatrix);

  mPosition   = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  mDir        = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  mRightVector      = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
  mUpVector         = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
  mLookVector       = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

  mSpeed = 0.0f;
  mCollMode = COLL_MODE_DISABLED;
}

void FpsCamera::setLens(float fov, float aspect, float nearZ, float farZ, float screenHeight, float screenWidth)
{
  D3DXMatrixPerspectiveFovLH(&mProjectionMatrix, fov, aspect, nearZ, farZ);	

  mScreenHeight = screenHeight;
  mScreenWidth = screenWidth;
}

void FpsCamera::update(const float dt)
{
  // check the direct input object	  

  if (mDInput->keyDown(DIK_W))
  {		
    mDir += mLookVector;
    mSpeed = min(400.0f, mSpeed + 50.0f);
  }
  else if (mDInput->keyDown(DIK_S))
  {		
    mDir -= mLookVector;
    mSpeed = min(400.0f, mSpeed + 50.0f);
  }
  else
  {
    mSpeed = max(0.0f, mSpeed - 30.0f);
  }

  if (mDInput->keyDown(DIK_D))
    mDir += mRightVector;

  if (mDInput->keyDown(DIK_A))
    mDir -= mRightVector;

  if (mCollMode == COLL_MODE_FULL)  
    mDir.y = 0.0f;

  D3DXVec3Normalize(&mDir, &mDir);

  // rotation
  float xDelta  = mDInput->mouseDeltaY() / mScreenHeight * 3;//200.0f;
  float yDelta = mDInput->mouseDeltaX() / mScreenWidth * 3;//200.0f;

  // do collision detection
  D3DXVECTOR3 oldPos = mPosition;

  mPosition += mDir*mSpeed*dt;	  

  const float PLAYER_HEIGHT = 50.0f;

  if (mCollMode == COLL_MODE_FULL)
  {  
    // lock to ground
    D3DXVECTOR3 yPos = mPosition + D3DXVECTOR3(0.0f, -PLAYER_HEIGHT, 0.0f);		
    mCollision->traceRay(mPosition, yPos);
    mCollision->getTraceResult(&yPos);	

    //if (fabs(yPos.y - mPosition.y) < PLAYER_HEIGHT)
    if (mPosition.y - yPos.y < PLAYER_HEIGHT)
    {
      mPosition.y += 7.0f;      
      //mPosition.y += fabs(yPos.y - mPosition.y);

      // clamp
      if (mPosition.y < (yPos.y + 1.0f))
        mPosition.y = yPos.y + 1.0f;
      if (mPosition.y > (yPos.y + PLAYER_HEIGHT))
        mPosition.y = yPos.y + PLAYER_HEIGHT;
    }

    // apply gravity
    mPosition += D3DXVECTOR3(0.0f, -4.0f, 0.0f);    

    // trace, adjust position
    mCollision->traceSphere(oldPos, mPosition, 10.0f); 
    mCollision->getTraceResult(&mPosition);	    
  }


  // build the camera matrices
  D3DXMATRIX Rotation;
  D3DXMatrixRotationAxis(&Rotation, &mRightVector, xDelta);
  D3DXVec3TransformCoord(&mLookVector, &mLookVector, &Rotation);
  D3DXVec3TransformCoord(&mUpVector, &mUpVector, &Rotation);

  D3DXMatrixRotationY(&Rotation, yDelta);
  D3DXVec3TransformCoord(&mRightVector, &mRightVector, &Rotation);
  D3DXVec3TransformCoord(&mUpVector, &mUpVector, &Rotation);
  D3DXVec3TransformCoord(&mLookVector, &mLookVector, &Rotation);

  buildViewMatrix();
  mViewFrustum->update(&mViewMatrix, &mProjectionMatrix);
  mViewProjMatrix = mViewMatrix * mProjectionMatrix;
}

void FpsCamera::setDInput(DInput *DInput)
{
  mDInput = DInput;
}

void FpsCamera::buildViewMatrix()
{	
  D3DXVec3Normalize(&mLookVector, &mLookVector);

  D3DXVec3Cross(&mUpVector, &mLookVector, &mRightVector);
  D3DXVec3Normalize(&mUpVector, &mUpVector);

  D3DXVec3Cross(&mRightVector, &mUpVector, &mLookVector);
  D3DXVec3Normalize(&mRightVector, &mRightVector);	

  float x = -D3DXVec3Dot(&mPosition, &mRightVector);
  float y = -D3DXVec3Dot(&mPosition, &mUpVector);
  float z = -D3DXVec3Dot(&mPosition, &mLookVector);

  mViewMatrix(0,0) = mRightVector.x; 
  mViewMatrix(1,0) = mRightVector.y; 
  mViewMatrix(2,0) = mRightVector.z; 
  mViewMatrix(3,0) = x;   

  mViewMatrix(0,1) = mUpVector.x;
  mViewMatrix(1,1) = mUpVector.y;
  mViewMatrix(2,1) = mUpVector.z;
  mViewMatrix(3,1) = y;  

  mViewMatrix(0,2) = mLookVector.x; 
  mViewMatrix(1,2) = mLookVector.y; 
  mViewMatrix(2,2) = mLookVector.z; 
  mViewMatrix(3,2) = z;   

  mViewMatrix(0,3) = 0.0f;
  mViewMatrix(1,3) = 0.0f;
  mViewMatrix(2,3) = 0.0f;
  mViewMatrix(3,3) = 1.0f;	
}

const D3DXVECTOR3* FpsCamera::position() const
{
  return &mPosition;
}

const D3DXMATRIX& FpsCamera::view() const
{
  return mViewMatrix;
}

const D3DXMATRIX& FpsCamera::projection() const
{
  return mProjectionMatrix;
}

const D3DXMATRIX& FpsCamera::viewProj(void) const
{
  return mViewProjMatrix;
}

void FpsCamera::setViewFrustum(ViewFrustum *viewFrustum)
{
  mViewFrustum = viewFrustum;	
}

void FpsCamera::setCollision(Collision *collision)
{
  mCollision = collision;
}

void FpsCamera::setCollMode(int mode)
{
  mCollMode = mode;
}
// OLD
//  // lock to ground
//D3DXVECTOR3 yPos = mPosition + D3DXVECTOR3(0.0f, -PLAYER_HEIGHT, 0.0f);		
//  mCollision->traceRay(mPosition, yPos);
//mCollision->getTraceResult(&yPos);	

//if (fabs(yPos.y - mPosition.y) < PLAYER_HEIGHT)
//	mPosition.y = yPos.y + PLAYER_HEIGHT;

//  // apply gravity
//mPosition += D3DXVECTOR3(0.0f, -5.0f, 0.0f);

//  // trace, adjust position
//mCollision->traceSphere(oldPos, mPosition, 10.0f); 
//mCollision->getTraceResult(&mPosition);	    

// NEW