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

// vertex.h -- vertex declarations used by the renderer

#ifndef VERTEX_H
#define VERTEX_H

#include <d3dx9.h>

struct LVertex
{
  LVertex(float _x, float _y, float _z, 
    float _nx, float _ny, float _nz,
    float _tu, float _tv,
    float _ltu, float _ltv): 
  x(_x), y(_y), z(_z),
    nx(_nx), ny(_ny), nz(_nz),
    tu(_tu), tv(_tv),
    ltu(_ltu), ltv(_ltv){}	
  float x;
  float y;
  float z;
  float nx;
  float ny;
  float nz;
  float tu, tv;
  float ltu, ltv;
};

struct LVertexSky {
  float x, y, z;
  float tu, tv;
};

const DWORD VertexFVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEX2 );
const DWORD VertexFVFSky = (D3DFVF_XYZ | D3DFVF_TEX1 );

void InitVertexDecl(IDirect3DDevice9* pd3dDevice);
void DeleteVertexDecl(void);

struct VertexPT
{
  VertexPT()
    :pos(0.0f, 0.0f, 0.0f),		
    tex0(0.0f, 0.0f){}
  VertexPT(float x, float y, float z, 		
    float u, float v):pos(x,y,z), tex0(u,v){}
  VertexPT(const D3DXVECTOR3& v, const D3DXVECTOR2& uv)
    :pos(v), tex0(uv){}

  D3DXVECTOR3 pos;	
  D3DXVECTOR2 tex0;

  static IDirect3DVertexDeclaration9* Decl;
};

struct VertexPNTL
{
  VertexPNTL()
    :pos(0.0f, 0.0f, 0.0f),
    normal(0.0f, 0.0f, 0.0f),
    tex0(0.0f, 0.0f),
    tex1(0.0f, 0.0f){}
  VertexPNTL(float x, float y, float z, 
    float nx, float ny, float nz,
    float u, float v,
    float lu, float lv):pos(x,y,z), normal(nx,ny,nz), tex0(u,v), tex1(lu,lv){}
  VertexPNTL(const D3DXVECTOR3& v, const D3DXVECTOR3& n, const D3DXVECTOR2& uv, const D3DXVECTOR3& lulv)
    :pos(v),normal(n), tex0(uv), tex1(lulv){}

  D3DXVECTOR3 pos;
  D3DXVECTOR3 normal;
  D3DXVECTOR2 tex0;
  D3DXVECTOR2 tex1;

  static IDirect3DVertexDeclaration9* Decl;
};
#endif // VERTEX_H