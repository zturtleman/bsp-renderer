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

// Renderer.cpp -- the heart of the application, handles the drawing of the map

#include "Renderer.h"
#include "FpsCamera.h"
#include "ZipHandler.h"
#include "Q3Map.h"
#include "BspVertex.h"
#include "vertex.h"
#include "misc.h"
#include "adaptation.h"

#include <iostream>
#include <tchar.h>

#include <d3d9.h>

using namespace std;

static unsigned int GetColour (unsigned int a, unsigned int r, unsigned int g, unsigned int b)
{
  //return (((a & 0xff)<<24) | ((r & 0xff)<<16) | ((g & 0xff)<<8) | (b & 0xff));
  return (((r & 0xff)<<16) | ((g & 0xff)<<8) | (b & 0xff));
}

Renderer::Renderer()
{
  mFpsCamera = new FpsCamera();
  mViewFrustum = new ViewFrustum();
  mCollision = new Collision();

  vec3f pos(0.0f, 0.0f, 0.0f);	

  mSetStreamAndIndices = true;
  mSetStreamAndIndices2 = true;

  mLastTexture = -1;
  mLastLightMap = -1;

  mNumBspFaces = 0;	
}

Renderer::~Renderer()
{
  DeleteVertexDecl();

  DELETE_P(mFpsCamera);
  DELETE_P(mViewFrustum);	
  DELETE_P(mCollision);

  DELETE_ARRAY(mFacesToRender);	

  if (mBspFaces)
  {
    for (int i=0; i<mNumBspFaces; i++)
    {
      if (mBspFaces[i].patch)
      {
        DELETE_ARRAY(mBspFaces[i].patch->bezier);
        DELETE_P(mBspFaces[i].patch);
      }
    }
  }

  DELETE_ARRAY(mBspFaces);
  DELETE_ARRAY(mD3DTextures);
  DELETE_ARRAY(mTextureLightMaps);

	ReleaseCOM(mSphere);
	ReleaseCOM(mEnvMap);	
}

void Renderer::setDInput(DInput *DInput)
{
  mDInput = DInput;
  mFpsCamera->setDInput(DInput);
}

void Renderer::setDXDevice(IDirect3DDevice9* d3dDevice)
{
  md3dDevice = d3dDevice;
}

void Renderer::update(const float dt)
{				
  mFpsCamera->update(dt);

  mNumFacesToRender = mQ3Map->findVisibleFaces(mFpsCamera->position(), mFacesToRender);
}

void Renderer::draw(void)
{		
  resetOptimisationVars();
  resetState();  

  // World matrix is identity.
  D3DXMATRIX W;
  D3DXMatrixIdentity(&W);
  V(md3dDevice->SetTransform(D3DTS_WORLD, &W));
  V(md3dDevice->SetTransform(D3DTS_VIEW, &mFpsCamera->view()));
  V(md3dDevice->SetTransform(D3DTS_PROJECTION, &mFpsCamera->projection()));	

  mFacesToSort = mFacesToRender;
  qsortFaces(0,  mNumFacesToRender - 1);

  int faceIndex = 0;	
  while (mFacesToRender[faceIndex] != -1)
  {
    if (mD3DTextures[mBspFaces[mFacesToRender[faceIndex]].texture] != NULL 
      && mBspFaces[mFacesToRender[faceIndex]].lm_index >= 0)
    {			
      drawFace(mFacesToRender[faceIndex]);			
    }
    faceIndex++;
  }			

  drawSky();
}

void Renderer::setMap(Q3Map *q3Map)
{
  mQ3Map = q3Map;		
}

void Renderer::initRenderer(void)
{
  mFacesToRender = new int[mQ3Map->m_iNumFaces + 1]; 

  initFaces();	

#ifndef NO_SHADERS
  InitVertexDecl(md3dDevice);
#endif

  mFpsCamera->setViewFrustum(mViewFrustum);
  mFpsCamera->setCollision(mCollision);
  mQ3Map->setViewFrustum(mViewFrustum);
  mCollision->setMap(mQ3Map);

  // load textures	
  mD3DTextures = new IDirect3DTexture9*[mQ3Map->m_iNumTexs];

  ZipHandler zip;	

  cout << "extracting " << mQ3Map->m_iNumTexs << " textures\n";

  // open all dirs

  for (int i=0; i<mQ3Map->m_iNumTexs; i++)
  {	
    if (zip.openDir(mQ3Map->pk3Files[0]))
    {
      if (zip.extractFile(string(mQ3Map->m_pTextures[i].name),_T("texture"), true))
      {
        cout << "adding texture " << mQ3Map->m_pTextures[i].name << " as number " << i << "\n";					
        V(D3DXCreateTextureFromFile(md3dDevice,_T("texture"), &mD3DTextures[i]));
      }
      else
      {
        cout << "couldn't find " << mQ3Map->m_pTextures[i].name << "\n"; 
        mD3DTextures[i] = NULL;
      }
      zip.closeDir(mQ3Map->pk3Files[0]);
    }
    else
    {
      cout << "could not open file " << mQ3Map->pk3Files[0] << " exiting...\n";
      exit(0);
    }
  }

  // load lightmaps	
  mTextureLightMaps = new IDirect3DTexture9*[mQ3Map->m_iNumLightMaps];

  for (int i=0; i < mQ3Map->m_iNumLightMaps; i++)
  {		
    V(D3DXCreateTexture(md3dDevice, 128, 128, 0, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &mTextureLightMaps[i]));

    D3DLOCKED_RECT pLockedRect;
    V(mTextureLightMaps[i]->LockRect(0, &pLockedRect, NULL, NULL));		

    void *pp;
    pp = pLockedRect.pBits;

    signed int *p;

    p = (signed int*) pp;

    Q3BspLightMap *lm;

    lm = &mQ3Map->m_pLightMaps[i];

    for (int x=0; x<128; ++x)
    {
      for (int y=0; y<128; ++y)
      {
        p[x*128 + y] = GetColour(255,
          lm->lightmap[x][y][0],
          lm->lightmap[x][y][1],
          lm->lightmap[x][y][2]);				
      }
    }
    mTextureLightMaps[i]->UnlockRect(0);
  }
}

void Renderer::buildIndexBuffer(void)
{		 	
  DEBUG_OUTPUT("total nr of meshverts: " << mQ3Map->m_iNumMeshVerts);

  DWORD usage;
#ifdef NO_SHADERS
  usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING;
#else
  usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
#endif

  // Obtain a pointer to a new index buffer.
  V(md3dDevice->CreateIndexBuffer(
    (UINT)(mQ3Map->m_iNumMeshVerts * sizeof(unsigned short)),		
    usage,
    D3DFMT_INDEX16,
    D3DPOOL_DEFAULT,
    &mIB,
    0));

  // Now lock it to obtain a pointer to its internal data, and write the
  // cube's index data.

  unsigned short* k = 0;

  V(mIB->Lock(0, 0, (void**)&k, 0));

  for (int i=0; i < mQ3Map->m_iNumMeshVerts; i++)
  {	
    k[i] = (unsigned short)mQ3Map->m_pMeshVerts[i];
  }

  V(mIB->Unlock());
}

void Renderer::buildVertexBuffer(void)
{
  // Obtain a pointer to a new vertex buffer.
  UINT vbLength;
  DWORD usage;
#ifdef NO_SHADERS
  vbLength = mQ3Map->m_iNumVertices * sizeof(LVertex);
  usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING;
#else
  vbLength = mQ3Map->m_iNumVertices * sizeof(VertexPNTL);
  usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
#endif

  V(md3dDevice->CreateVertexBuffer(
    vbLength,    
    usage,
    VertexFVF,
    D3DPOOL_DEFAULT,
    &mVB,
    0));

  // Now lock it to obtain a pointer to its internal data, and write the
  // cube's vertex data.

#ifdef NO_SHADERS
  LVertex* v = 0;
#else
  VertexPNTL* v = 0;
#endif
    
  V(mVB->Lock(0, 0, (void**)&v, 0));

  Q3BspVertex *vertices = mQ3Map->m_pVertices;

  for (int k = 0; k < mQ3Map->m_iNumVertices; k++)
  {		
#ifdef NO_SHADERS
    v[k] = LVertex(vertices[k].position[0], 
                   vertices[k].position[1],
                   vertices[k].position[2],
                   vertices[k].normal[0],
                   vertices[k].normal[1],
                   vertices[k].normal[2],
                   vertices[k].texcoord[0][0],
                   vertices[k].texcoord[0][1],
                   vertices[k].texcoord[1][0],
                   vertices[k].texcoord[1][1]);
#else
    v[k] = VertexPNTL(vertices[k].position[0], 
                   vertices[k].position[1],
                   vertices[k].position[2],
                   vertices[k].normal[0],
                   vertices[k].normal[1],
                   vertices[k].normal[2],
                   vertices[k].texcoord[0][0],
                   vertices[k].texcoord[0][1],
                   vertices[k].texcoord[1][0],
                   vertices[k].texcoord[1][1]);
#endif
  }

  V(mVB->Unlock());
}

void Renderer::buildPatchBuffers(void)
{	
  int numIndex = 0;
  int numVertex = 0;

  if (mBspFaces != NULL)
  {
    for (int i=0; i < mNumBspFaces; i++)
    {					
      if (mBspFaces[i].type == PATCH)
      {
        Q3BspPatch *patch = mBspFaces[i].patch;
        if (patch != NULL)
        {
          for (int j=0; j < patch->size; j++)
          {
            numIndex += patch->bezier[j].mNumIndex;
            numVertex += patch->bezier[j].mNumVertex;
          }
        }
      }
    }

    if (numVertex == 0 || numIndex == 0)
      return;

    // create index buffer		
    DWORD usage;
#ifdef NO_SHADERS
    usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING;
#else
    usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
#endif
    V(md3dDevice->CreateIndexBuffer(
      (UINT)numIndex * sizeof(unsigned short),
      usage,
      D3DFMT_INDEX16,
      D3DPOOL_DEFAULT,
      &mBezIB,
      0));

    // create vertex buffer
    UINT vbLength;
    DWORD FVF;

#ifdef NO_SHADERS
    vbLength = numVertex * sizeof(LVertex);
    FVF = VertexFVF;
    usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING;
#else
    vbLength = numVertex * sizeof(VertexPNTL);
    FVF = 0;
    usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
#endif

    V(md3dDevice->CreateVertexBuffer(
      vbLength,
      usage,
      FVF,
      D3DPOOL_DEFAULT,
      &mBezVB,
      0));

    unsigned short *k = 0;
    V(mBezIB->Lock(0, 0, (void**)&k, D3DLOCK_DISCARD ));		
    int indexBufferindex = 0;

#ifdef NO_SHADERS
    LVertex* v = 0;	
#else
    VertexPNTL* v = 0; 
#endif
    V(mBezVB->Lock(0, 0, (void**)&v, D3DLOCK_DISCARD));		
    int vertexBufferindex = 0;

    for (int faceIndex=0; faceIndex < mNumBspFaces; faceIndex++)
    {						
      if (mBspFaces[faceIndex].type == PATCH)
      {
        Q3BspPatch *patch = mBspFaces[faceIndex].patch;
        if (patch != NULL)
        {
          for (int bezierIndex=0; bezierIndex < patch->size; bezierIndex++)
          {
            patch->bezier[bezierIndex].mBaseBufferindex = indexBufferindex;

            for (unsigned int index=0; index < patch->bezier[bezierIndex].mNumIndex; index++)
            {	
              k[indexBufferindex] = (unsigned short)patch->bezier[bezierIndex].mIndex[index];
              indexBufferindex++;
            }

            patch->bezier[bezierIndex].mBaseVertexIndex = vertexBufferindex;

            for (unsigned int vertex=0; vertex < patch->bezier[bezierIndex].mNumVertex; vertex++)
            {
              BspVertex *bspVertex = &patch->bezier[bezierIndex].mVertex[vertex];
#ifdef NO_SHADERS
              v[vertexBufferindex] = LVertex(
                //position									  
                bspVertex->mPosition[0],
                bspVertex->mPosition[1],
                bspVertex->mPosition[2],
                // normal
                bspVertex->mNormal[0],
                bspVertex->mNormal[1],
                bspVertex->mNormal[2],
                // texture coordinates									  
                bspVertex->mTexcoord[0][0],
                bspVertex->mTexcoord[0][1],
                // lightmap coordinates
                bspVertex->mTexcoord[1][0],
                bspVertex->mTexcoord[1][1]);
#else
              v[vertexBufferindex] = VertexPNTL(
                //position									  
                bspVertex->mPosition[0],
                bspVertex->mPosition[1],
                bspVertex->mPosition[2],
                // normal
                bspVertex->mNormal[0],
                bspVertex->mNormal[1],
                bspVertex->mNormal[2],
                // texture coordinates									  
                bspVertex->mTexcoord[0][0],
                bspVertex->mTexcoord[0][1],
                // lightmap coordinates
                bspVertex->mTexcoord[1][0],
                bspVertex->mTexcoord[1][1]);
#endif
              vertexBufferindex++;
            }
          }
        }
      }
    }

    V(mBezIB->Unlock());
    V(mBezVB->Unlock());						

  }
}

void Renderer::drawFace(int faceIndex)
{						
  // lightmaps and textures							
  if (mLastLightMap != mBspFaces[faceIndex].lm_index)
  {
    V(md3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1));
    V(md3dDevice->SetTexture(0, mTextureLightMaps[mBspFaces[faceIndex].lm_index]));
    mLastLightMap = mBspFaces[faceIndex].lm_index;
  }

  if (mLastTexture != mBspFaces[faceIndex].texture)
  {
    V(md3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0));
    V(md3dDevice->SetTexture(1, mD3DTextures[mBspFaces[faceIndex].texture]));
    mLastTexture = mBspFaces[faceIndex].texture;
  }

  switch (mBspFaces[faceIndex].type)
  {
  case MESH:
    break;
  case POLYGON:		
    if (mSetStreamAndIndices)
    {			
#ifdef NO_SHADERS
      V(md3dDevice->SetStreamSource(0, mVB, 0, sizeof(LVertex)));
#else
      V(md3dDevice->SetStreamSource(0, mVB, 0, sizeof(VertexPNTL)));
#endif
      V(md3dDevice->SetIndices(mIB));

      mSetStreamAndIndices = false;
    }

    mSetStreamAndIndices2 = true;

    V(md3dDevice->DrawIndexedPrimitive(
      D3DPT_TRIANGLELIST, 
      mBspFaces[faceIndex].vertex, 
      0,
      mBspFaces[faceIndex].n_vertexes,
      mBspFaces[faceIndex].meshvert,
      mBspFaces[faceIndex].n_triangles));						

    break;

  case PATCH:

    if (mSetStreamAndIndices2)
    {
#ifdef NO_SHADERS
      V(md3dDevice->SetStreamSource(0, mBezVB, 0, sizeof(LVertex)));
#else
      V(md3dDevice->SetStreamSource(0, mBezVB, 0, sizeof(VertexPNTL)));
#endif
      V(md3dDevice->SetIndices(mBezIB));		
      mSetStreamAndIndices2 = false;
    }
    mSetStreamAndIndices = true;

    for (int i=0; i<mBspFaces[faceIndex].patch->size; i++)
    {						
      for (int j=0; j < 5; j++)
      {
        V(md3dDevice->DrawIndexedPrimitive(
          D3DPT_TRIANGLESTRIP,
          mBspFaces[faceIndex].patch->bezier[i].mBaseVertexIndex,//0,
          0,
          (UINT)mBspFaces[faceIndex].patch->bezier[i].mNumVertex,
          mBspFaces[faceIndex].patch->bezier[i].mRowIndex[j] + mBspFaces[faceIndex].patch->bezier[i].mBaseBufferindex,
          mBspFaces[faceIndex].patch->bezier[i].mTrianglesPerRow[j]-2)); 
      }
    }

    break;
    //case BILLBOARD:
    //	break;
  default:
    break;		
  }	

}

void Renderer::setupState(void)
{	  
  V(md3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE ));

  V(md3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE));	

  V(md3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));

#ifdef NO_SHADERS
  V(md3dDevice->SetFVF(VertexFVF));  
#else
  V(md3dDevice->SetVertexDeclaration(VertexPNTL::Decl));		
#endif

  V(md3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, TEXTURE_QUALITY)); 
  V(md3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, TEXTURE_QUALITY)); 
  V(md3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, TEXTURE_QUALITY));		

  V(md3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, TEXTURE_QUALITY)); 
  V(md3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, TEXTURE_QUALITY)); 
  V(md3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, TEXTURE_QUALITY));	

  V(md3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ));	

  V(md3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,  D3DTOP_MODULATE4X));
  V(md3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,  D3DTOP_SELECTARG1));

  V(md3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,  D3DTOP_DISABLE ));
  V(md3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,  D3DTOP_DISABLE ));		

  V(md3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU));
  V(md3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE ));	
}

void Renderer::resetOptimisationVars()
{
  mSetStreamAndIndices = true;
  mSetStreamAndIndices2 = true;

  mLastTexture = -1;
  mLastLightMap = -1;
}

void Renderer::onResetDevice(D3DPRESENT_PARAMETERS *md3dPP)
{
  // The aspect ratio depends on the backbuffer dimensions, which can 
  // possibly change after a reset.  So rebuild the projection matrix.
  float w = (float)md3dPP->BackBufferWidth;
  float h = (float)md3dPP->BackBufferHeight;
  mFpsCamera->setLens(D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f, h, w);		

  mSetStreamAndIndices = true;
  mSetStreamAndIndices2 = true;

  mLastLightMap = -1;
  mLastTexture = -1;

  buildVertexBuffer();

  buildIndexBuffer();

  buildPatchBuffers();	
}

void Renderer::onLostDevice(void)
{
  ReleaseCOM(mIB);
  ReleaseCOM(mVB);
  ReleaseCOM(mBezIB);
  ReleaseCOM(mBezVB);	
}

void Renderer::initFaces(void)
{
  Q3BspFace_t *faces = mQ3Map->getFaces();

  mBspFaces = new Q3BspFaceRenderer[mQ3Map->m_iNumFaces];
  mNumBspFaces = mQ3Map->m_iNumFaces;

  for (int i=0; i < mNumBspFaces; i++)
  {					
    mBspFaces[i].lm_index = faces[i].lm_index;
    mBspFaces[i].meshvert = faces[i].meshvert;
    mBspFaces[i].n_meshverts = faces[i].n_meshverts;
    mBspFaces[i].n_vertexes = faces[i].n_vertexes;
    for (int j=0; j<3; j++)
      mBspFaces[i].normal[j] = faces[i].normal[j];
    mBspFaces[i].texture = faces[i].texture;
    mBspFaces[i].type = faces[i].type;
    mBspFaces[i].vertex = faces[i].vertex;

    mBspFaces[i].n_triangles = mBspFaces[i].n_meshverts / 3;

    if (mBspFaces[i].type == PATCH)
    {
      mBspFaces[i].patch = mQ3Map->handlePatch(i);
    }
    else
    {
      mBspFaces[i].patch = NULL;
    }
  }
}


void Renderer::qsortFaces(int start, int end)
{
  if (start < end)
  {
    int elem = qsortPartition(start, end);
    qsortFaces( start, elem - 1);
    qsortFaces( elem + 1, end);
  }
}

int Renderer::qsortPartition(int start, int end)
{
  int texture = mBspFaces[mFacesToSort[end]].texture;
  int lm_index = mBspFaces[mFacesToSort[end]].lm_index;
  int type = mBspFaces[mFacesToSort[end]].type;
  int i = start - 1;

  for (int j=start; j<end; j++)
  {
    if (mBspFaces[mFacesToSort[j]].texture == texture && 
      mBspFaces[mFacesToSort[j]].lm_index == lm_index &&
      mBspFaces[mFacesToSort[j]].type <= type)
    {
      i++;
      swapFaces( i, j);
    }
    else if (mBspFaces[mFacesToSort[j]].texture == texture && 
      mBspFaces[mFacesToSort[j]].lm_index <= lm_index)
    {
      i++;
      swapFaces( i, j);
    }
    else if (mBspFaces[mFacesToSort[j]].texture < texture)
    {
      i++;
      swapFaces( i, j);
    }	
  }

  swapFaces(i+1,end);

  return i+1;
}

void Renderer::swapFaces(int i, int j)
{
  int temp = mFacesToSort[i];
  mFacesToSort[i] = mFacesToSort[j];
  mFacesToSort[j] = temp;
}


void Renderer::createSkyFX(void)
{
  float skyRadius = 4500.0f;

  V(D3DXCreateSphere(md3dDevice, skyRadius, 3, 3, &mSphere, 0));		
  V(D3DXCreateCubeTextureFromFileEx(
    md3dDevice,
    _T("media/sky3.dds"),
    D3DX_DEFAULT,
    1,
    0,
    D3DFMT_X1R5G5B5,
    D3DPOOL_MANAGED,
    D3DX_FILTER_NONE,
    D3DX_FILTER_NONE,
    0,
    NULL,
    NULL,
    &mEnvMap));	
}

void Renderer::drawSky(void)
{
#define SKY_TEXTURE_QUALITY D3DTEXF_ANISOTROPIC
  V(md3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, SKY_TEXTURE_QUALITY)); 
  V(md3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, SKY_TEXTURE_QUALITY)); 
  V(md3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, SKY_TEXTURE_QUALITY));		

  V(md3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,  D3DTOP_DISABLE ));
  V(md3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,  D3DTOP_DISABLE ));		

  D3DXMATRIX matEnvironmentAdjust;

#ifdef NO_SHADERS
  V(md3dDevice->SetFVF(VertexFVFSky));
#else
  V(md3dDevice->SetVertexDeclaration(VertexPT::Decl));	
#endif
  
  D3DXMATRIX view, view2;
  view = mFpsCamera->view();
  view2 = view;
  view2(3,0) = 0.0f;
  view2(3,1) = 0.0f;
  view2(3,2) = 0.0f;	

  D3DXMatrixInverse(&matEnvironmentAdjust, NULL, &view2);
  matEnvironmentAdjust = -matEnvironmentAdjust;
  md3dDevice->SetTransform(D3DTS_TEXTURE0, &matEnvironmentAdjust);

  md3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
  md3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 );

  md3dDevice->SetTexture(0, mEnvMap);	

  D3DXMATRIX W;
  D3DXMatrixIdentity(&W);	
  V(md3dDevice->SetTransform(D3DTS_VIEW, &view2));	
  V(mSphere->DrawSubset(0));	
}

void Renderer::resetState()
{
#ifdef NO_SHADERS
  V(md3dDevice->SetFVF(VertexFVF));
#else
  V(md3dDevice->SetVertexDeclaration(VertexPNTL::Decl));		
#endif
  
  V(md3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, TEXTURE_QUALITY)); 
  V(md3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, TEXTURE_QUALITY)); 
  V(md3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, TEXTURE_QUALITY));

  V(md3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,  D3DTOP_MODULATE4X));
  V(md3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,  D3DTOP_SELECTARG1));

  V(md3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0));
  V(md3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE ));	
}

void Renderer::setCollMode(int mode)
{
  mFpsCamera->setCollMode(mode);
}