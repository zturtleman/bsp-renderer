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

// Renderer.h -- the heart of the application, handles the drawing of the map

#ifndef _RENDERER_H
#define _RENDERER_H

#include <vector>

#include "FpsCamera.h"
#include "Q3Map.h"
#include "Bezier.h"
#include "misc.h"
#include "Collision.h"

#define TEXTURE_QUALITY /*D3DTEXF_LINEAR*/ D3DTEXF_ANISOTROPIC 

enum {R_COLL_MODE_DISABLED, R_COLL_MODE_NO_GRAVITY, R_COLL_MODE_GRAVITY};

#ifdef NO_HW_VERTEXPROCESSING
#define VB_INDEX_TYPE unsigned short
#define VB_FORMAT D3DFMT_INDEX16
#else
#define VB_INDEX_TYPE unsigned int
#define VB_FORMAT D3DFMT_INDEX32
#endif

class Renderer
{
public:
  Renderer();
  ~Renderer();

  void setDInput(DInput *DInput);
  void setDXDevice(IDirect3DDevice9* d3dDevice);
  void setMap(Q3Map *q3Map);
  void initRenderer(void);
  void setupState(void);
  void onResetDevice(D3DPRESENT_PARAMETERS *md3dPP);
  void onLostDevice(void);
  void update(const float dt);
  void draw(void);
  void createSkyFX(void);
  void setCollMode(int mode);  

private:	

  typedef struct{
    int texture;    
    int type;
    int vertex;
    int n_vertexes;
    int meshvert;
    int n_meshverts;
    int n_triangles;
    int lm_index;    
    float normal[3];    
    Q3BspPatch *patch;
  } Q3BspFaceRenderer;

  void buildIndexBuffer(void);
  void buildVertexBuffer(void);
  void initFaces(void);
  void buildPatchBuffers(void);
  void resetOptimisationVars(void);

  void drawFace(int faceIndex);
  void drawSky(void);
  void resetState(void);

  FpsCamera *mFpsCamera;
  DInput *mDInput;
  IDirect3DDevice9* md3dDevice;
  ViewFrustum *mViewFrustum;
  Collision *mCollision;

  Q3Map *mQ3Map;

  // textures	
  IDirect3DTexture9** mD3DTextures; 	
  // lightmaps	
  IDirect3DTexture9** mTextureLightMaps;

  IDirect3DVertexBuffer9* mVB;
  IDirect3DIndexBuffer9*  mIB;
  IDirect3DVertexBuffer9* mBezVB;
  IDirect3DIndexBuffer9*  mBezIB;

  int *mFacesToRender;
  int mNumFacesToRender;
  Q3BspFaceRenderer *mBspFaces;	
  int mNumBspFaces;

  // variables to avoid setting state if not necessary
  bool mSetStreamAndIndices;
  bool mSetStreamAndIndices2;
  int mLastLightMap;
  int mLastTexture;

  // sort the faces before rendering
  void qsortFaces(int start, int end);
  int qsortPartition(int start, int end);
  void swapFaces(int i, int j);
  int *mFacesToSort;

  // sky box
  ID3DXMesh* mSphere;
  IDirect3DCubeTexture9* mEnvMap;  
};

#endif /* _RENDERER_H */