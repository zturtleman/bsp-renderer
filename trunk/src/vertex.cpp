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

// vertex.cpp -- vertex declarations used by the renderer

#include <d3d9.h>

#include "vertex.h"
#include "misc.h"

IDirect3DVertexDeclaration9* VertexPNTL::Decl = 0;
IDirect3DVertexDeclaration9* VertexPT::Decl = 0;

void InitVertexDecl(IDirect3DDevice9* pd3dDevice)
{
  D3DVERTEXELEMENT9 VertexPNTLElements[] = 
  {
    {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
    D3DDECL_END()
  };	
  V(pd3dDevice->CreateVertexDeclaration(VertexPNTLElements, &VertexPNTL::Decl));

  D3DVERTEXELEMENT9 VertexPTElements[] = 
  {
    {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},		
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
  };	
  V(pd3dDevice->CreateVertexDeclaration(VertexPTElements, &VertexPT::Decl));
}

void DeleteVertexDecl(void)
{
  ReleaseCOM(VertexPNTL::Decl);
  ReleaseCOM(VertexPT::Decl);	
}