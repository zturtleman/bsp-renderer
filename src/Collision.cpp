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

// Some of the code in this module is directly based on code from 
// Quake III Arena, which is Copyright (C) 1999-2005 Id Software, Inc.
// The Quake III Arena source code is released under the GPL v2 license.

#include "Collision.h"
#include "Q3Map.h"

Collision::Collision()
{
  mOutputEnd.x = 0.0f;
  mOutputEnd.y = 0.0f;
  mOutputEnd.z = 0.0f;
}

void Collision::setMap(Q3Map *q3Map)
{
  mQ3Map = q3Map;
}

void Collision::traceSphere(vec3f start, vec3f end, float radius)
{
  mTraceType = TT_SPHERE;
  mTraceRadius = radius;
  mInputStart = start;
  mInputEnd = end;

  trace();
}

void Collision::traceRay(vec3f start, vec3f end)
{
  mInputStart = start;
  mInputEnd = end;
  mOutputFraction = 1.0f;
  mGoodPos = true;
  mTraceType = TT_RAY;

  CheckNode( 0, 0.0f, 1.0f, mInputStart, mInputEnd );

  if (mGoodPos)
  {
    mOutputEnd = mInputEnd;
  }
  else
  {
    mOutputEnd = mInputStart + mOutputFraction*(mInputEnd - mInputStart);
  }
}

void Collision::getTraceResult(vec3f *output)
{
  output->x = mOutputEnd.x;
  output->y = mOutputEnd.y;
  output->z = mOutputEnd.z;
}

void Collision::trace()
{		
  int numTries = 0;

  while (numTries < 3)
  {		
    mOutputFraction = 1.0f;
    mGoodPos = true;
    CheckNode( 0, 0.0f, 1.0f, mInputStart, mInputEnd );
    if (mGoodPos)
    {
      mOutputEnd = mInputEnd;
      return;
    }
    else
    {
      mInputEnd = mInputEnd + mCollisionNormal*(-mEndDistance + EPSILON);
      numTries++;
    }		
  }
  // did not find a good position
  mOutputEnd = mInputStart;	
}

void Collision::CheckNode( int nodeIndex, float startFraction, float endFraction, vec3f start, vec3f end)
{
  if (nodeIndex < 0)
  {	
    // this is a leaf
    Q3BspLeaf *leaf = &mQ3Map->m_pLeafs[-(nodeIndex + 1)];

    for (int i = 0; i < leaf->n_leafbrushes; i++)
    {
      Q3BspBrush *brush = &mQ3Map->m_pBrushes[mQ3Map->m_pLeafBrushes[leaf->leafbrush + i]];
      if (brush->n_brushsides > 0 && (mQ3Map->m_pTextures[brush->texture].contents & 1) )			    
      {
        CheckBrush( brush );
      }
    }

    // don't have to do anything else for leaves
    return;
  }

  // this is a node

  Q3BspNode *node = &mQ3Map->m_pNodes[nodeIndex];
  Q3BspPlane *plane = &mQ3Map->m_pPlanes[node->plane];

  vec3f normal = vec3f(plane->normal); 
  vec3f startDX = vec3f(start);
  vec3f endDX = vec3f(end); 

  float startDistance = vec3dot( &startDX, &normal ) - plane->dist;
  float endDistance = vec3dot( &endDX, &normal ) - plane->dist;
  float offset;

  if (mTraceType == TT_RAY)
  {
    offset = 0.0f;
  }
  else if (mTraceType == TT_SPHERE)
  {
    offset = mTraceRadius;
  }
  else
  {
    offset = 0.0f;
  }

  if (startDistance >= offset && endDistance >= offset)
  {	// both points are in front of the plane
    // so check the front child
    CheckNode( node->children[0], startFraction, endFraction, start, end );
  }
  else if (startDistance < -offset && endDistance < -offset)
  {	// both points are behind the plane
    // so check the back child
    CheckNode( node->children[1], startFraction, endFraction, start, end );
  }
  else
  {	// the line spans the splitting plane
    int side;
    float fraction1, fraction2, middleFraction;
    vec3f middle;

    // split the segment into two
    if (startDistance < endDistance)
    {
      side = 1; // back
      float inverseDistance = 1.0f / (startDistance - endDistance);
      fraction1 = (startDistance - offset + EPSILON) * inverseDistance;
      fraction2 = (startDistance + offset + EPSILON) * inverseDistance;
    }
    else if (endDistance < startDistance)
    {
      side = 0; // front
      float inverseDistance = 1.0f / (startDistance - endDistance);
      fraction1 = (startDistance + offset + EPSILON) * inverseDistance;
      fraction2 = (startDistance - offset - EPSILON) * inverseDistance;
    }
    else
    {
      side = 0; // front
      fraction1 = 1.0f;
      fraction2 = 0.0f;
    }

    // make sure the numbers are valid
    if (fraction1 < 0.0f) fraction1 = 0.0f;
    else if (fraction1 > 1.0f) fraction1 = 1.0f;
    if (fraction2 < 0.0f) fraction2 = 0.0f;
    else if (fraction2 > 1.0f) fraction2 = 1.0f;

    // calculate the middle point for the first side
    middleFraction = startFraction + (endFraction - startFraction) * fraction1;
    /*for (int i = 0; i < 3; i++)
    middle[i] = start[i] + fraction1 * (end[i] - start[i]);*/
    middle = start + fraction1 * (end - start);

    // check the first side
    CheckNode( node->children[side], startFraction, middleFraction, start, middle );

    // calculate the middle point for the second side
    middleFraction = startFraction + (endFraction - startFraction) * fraction2;
    /*for (int i = 0; i < 3; i++)
    middle[i] = start[i] + fraction2 * (end[i] - start[i]);*/
    middle = start + fraction2 * (end - start);

    // check the second side
    CheckNode( node->children[!side], middleFraction, endFraction, middle, end );
  }
}

void Collision::CheckBrush( Q3BspBrush *brush )
{
  float startFraction = -1.0f;
  float endFraction = 1.0f;
  bool startsOut = false;
  bool endsOut = false;
  vec3f collNormal;
  float endDistSaved = 0.0f;

  collNormal = vec3f(0.0f, 0.0f, 0.0f);

  for (int i = 0; i < brush->n_brushsides; i++)
  {
    Q3BspBrushSide *brushSide = &mQ3Map->m_pBrushSides[brush->brushside + i];
    Q3BspPlane *plane = &mQ3Map->m_pPlanes[brushSide->plane];

    float startDistance, endDistance;
    vec3f planeNormal = vec3f(plane->normal);

    if (mTraceType == TT_RAY)
    {    
      startDistance = vec3dot( &mInputStart, &planeNormal ) - plane->dist;
      endDistance = vec3dot( &mInputEnd, &planeNormal ) - plane->dist;
    }
    else /*if (mTraceType == TT_SPHERE)*/
    {
      startDistance = vec3dot( &mInputStart, &planeNormal ) - (plane->dist + mTraceRadius);
      endDistance = vec3dot( &mInputEnd, &planeNormal ) - (plane->dist + mTraceRadius);			
    }

    if (startDistance > 0)
      startsOut = true;
    if (endDistance > 0)
      endsOut = true;

    // make sure the trace isn't completely on one side of the brush
    if (startDistance > 0 && endDistance > 0)
    {   // both are in front of the plane, its outside of this brush
      return;
    }
    if (startDistance <= 0 && endDistance <= 0)
    {   // both are behind this plane, it will get clipped by another one
      continue;
    }

    if (startDistance > endDistance)
    {   // line is entering into the brush
      float fraction = (startDistance - EPSILON) / (startDistance - endDistance);
      if (fraction < 0)
        fraction = 0;
      if (fraction > startFraction)
      {
        startFraction = fraction;
        collNormal = vec3f(plane->normal);
        endDistSaved = endDistance;
      }
    }
    else
    {   // line is leaving the brush
      float fraction = (startDistance + EPSILON) / (startDistance - endDistance);
      if (fraction < endFraction)
        endFraction = fraction;
    }
  }

  if (startsOut == false)
  {
    mOutputStartOut = false;
    if (endsOut == false)
      mOutputAllSolid = true;
    return;
  }

  if (startFraction < endFraction)
  {
    if (startFraction > -1 && startFraction < mOutputFraction)
    {
      if (startFraction < 0)
        startFraction = 0;

      mGoodPos = false;
      mOutputFraction = startFraction;
      mCollisionNormal = collNormal;
      mEndDistance = endDistSaved;
    }
  }
}