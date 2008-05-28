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

// Collision.cpp -- collision handling and detection

#ifndef _COLLISION_H
#define _COLLISION_H

#include "Q3Map.h"
#include "adaptation.h"

enum { TT_RAY, TT_SPHERE };
#define EPSILON	(float) 1/32

class Collision
{
public:
  Collision();
  void setMap(Q3Map *q3Map);
  void traceSphere(vec3f start, vec3f end, float radius);
  void traceRay(vec3f start, vec3f end);
  void getTraceResult(vec3f *output);

private:
  void trace(void);
  void CheckNode( 
    int nodeIndex,
    float startFraction,
    float endFraction,
    vec3f start,
    vec3f end);
  void CheckBrush( Q3BspBrush *brush );

  Q3Map *mQ3Map;

  float mOutputFraction;
  vec3f mOutputEnd;
  bool mOutputStartOut;
  bool mOutputAllSolid;

  vec3f mInputStart;
  vec3f mInputEnd;
  vec3f mCollisionNormal;
  float mEndDistance;

  int mTraceType;
  float mTraceRadius;

  bool mGoodPos;
};

#endif /* _COLLISION_H */