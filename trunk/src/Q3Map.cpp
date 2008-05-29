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

// Q3Map.cpp -- handles the map data

#include <string>
#include <fstream>
#include <iostream>

#include "Q3Map.h"
#include "ZipHandler.h"
#include "FpsCamera.h"
#include "misc.h"

#include <d3dx9.h>

using namespace std;

Q3Map::Q3Map()
{
  // nothing here
}

Q3Map::~Q3Map()
{
  DELETE_P(m_pTextures);
  DELETE_P(m_pFaces);
  DELETE_P(m_pVertices);
  DELETE_P(m_pMeshVerts);
  DELETE_P(m_pLeafs);
  DELETE_P(m_pLeafFaces);
  DELETE_P(m_pPlanes);
  DELETE_P(m_pNodes);
  if (m_VisData != NULL)
  {
    DELETE_P(m_VisData->vecs);
    DELETE_P(m_VisData);
  }
  //DELETE_P(m_pVisibleFaces);		
  //DELETE_P(m_pFacesToRender);		
  DELETE_P(m_pBrushes);
  DELETE_P(m_pLeafBrushes);
  DELETE_P(m_pBrushSides);
  DELETE_P(m_pLightMaps);	
  DELETE_P(m_pEntities);

  delete mVisibleFaces;

}

void Q3Map::loadMap(string cfgFile)
{
  ifstream mapFile;

  mapFile.open(_T(cfgFile.c_str()));

  if (!mapFile) 
  {
    cout << "Unable to open file " << cfgFile << "\n";    
  }
  else
  {
    string str;

    const int tmp_length = 150;
    char tmp[tmp_length];

    // todo: clean up the parsing
    while (mapFile >> str) 
    {
      if (str == "#")
      {
        mapFile.getline(tmp,sizeof(tmp));
      }
      else if (str == "pk3:")
      {	
        do
        {
          mapFile.read(tmp, 1);
        }
        while (tmp[0] == ' ' && tmp[0]!= '\n');

        mapFile.unget();
        mapFile.getline(tmp, sizeof(tmp));

        pk3Files.push_back(tmp);			
        cout << "pk3 file: " << tmp << "\n" ; 
      }
      else if(str == "map:")
      {
        mapFile >> mapName;
        cout << "map: " << mapName << "\n";
      }
    }

    mapFile.close();	
  }

  /* If no pk3 file name is found, use the default */
  if (pk3Files.size() == 0)
  {
    pk3Files.push_back("maps/jof3dm2/jof3dm2.pk3");
    mapName = "maps/jof3dm2.bsp";
  }

  initMap();
}

void Q3Map::parseMap()
{	
  ZipHandler zip;

  if (!zip.openDir(pk3Files[0]))
    exitFunction("pk3 file not found");

  //if (!zip.openDir(pk3Files[1]))
  //exitFunction("pk3 file not found");

  if (!zip.extractFile(mapName, _T("map.bsp"), false))
    exitFunction("map file not found");

  zip.closeDir(pk3Files[0]);

  ifstream mapFile;

  mapFile.open(_T("map.bsp"), ios::in | ios::binary);

  if( !mapFile )
  {
    cout << "error opening BSP map!\n Exit...\n" ;
    exit(1);
  }

  mapFile.read((char*)&m_BspHeader, sizeof(Q3BspHeader_t));	

  DEBUG_OUTPUT("magic number: " << m_BspHeader.magic);	

  // Allocate memory
  m_pEntities = new char[m_BspHeader.Lumps[0].iLength];

  m_iNumFaces = m_BspHeader.Lumps[Faces].iLength / sizeof(Q3BspFace_t);
  m_pFaces = new Q3BspFace_t[m_iNumFaces];

  m_iNumVertices = m_BspHeader.Lumps[Vertices].iLength / sizeof(Q3BspVertex);
  m_pVertices = new Q3BspVertex[m_iNumVertices];

  m_iNumMeshVerts = m_BspHeader.Lumps[MeshVerts].iLength / sizeof(int);
  m_pMeshVerts = new int[m_iNumMeshVerts];

  m_iNumLeafs = m_BspHeader.Lumps[Leafs].iLength / sizeof(Q3BspLeaf);
  m_pLeafs = new Q3BspLeaf[m_iNumLeafs];

  m_iNumLeafFaces = m_BspHeader.Lumps[LeafFaces].iLength / sizeof(int);
  m_pLeafFaces = new int[m_iNumLeafFaces];

  m_iNumLeafBrushes = m_BspHeader.Lumps[LeafBrushes].iLength / sizeof(int);
  m_pLeafBrushes = new int[m_iNumLeafBrushes];

  m_iNumTexs = m_BspHeader.Lumps[Textures].iLength / sizeof(Q3BspTexture);
  m_pTextures = new Q3BspTexture[m_iNumTexs];

  m_iNumPlanes = m_BspHeader.Lumps[Planes].iLength / sizeof(Q3BspPlane);
  m_pPlanes = new Q3BspPlane[m_iNumPlanes];

  m_iNumNodes = m_BspHeader.Lumps[Nodes].iLength / sizeof(Q3BspNode);
  m_pNodes = new Q3BspNode[m_iNumNodes];

  //m_iNumModels = m_BspHeader.Lumps[Models].iLength / sizeof(Q3BspModel);
  //m_pModels = new Q3BspModel[m_iNumModels];

  m_iNumLightMaps = m_BspHeader.Lumps[LightMaps].iLength / sizeof(Q3BspLightMap);
  m_pLightMaps = new Q3BspLightMap[m_iNumLightMaps];

  m_iNumBrushes = m_BspHeader.Lumps[Brushes].iLength / sizeof(Q3BspBrush);
  m_pBrushes = new Q3BspBrush[m_iNumBrushes];

  m_iNumBrushSides = m_BspHeader.Lumps[BrushSides].iLength / sizeof(Q3BspBrushSide);
  m_pBrushSides = new Q3BspBrushSide[m_iNumBrushSides];

  //m_iNumEffects = m_BspHeader.Lumps[Effects].iLength / sizeof(Q3BspEffect);
  //m_pEffects = new Q3BspEffect[m_iNumEffects];
  //
  //m_pImages = new BDTexture[m_iNumTexs];

  m_VisData = new Q3BspVisData;

  // Read in data	

  mapFile.seekg(m_BspHeader.Lumps[0].iOffset);
  mapFile.read(m_pEntities, m_BspHeader.Lumps[0].iLength );

  //DEBUG_OUTPUT(m_pEntities);

  DEBUG_OUTPUT("number of faces: " << m_iNumFaces);
  DEBUG_OUTPUT("parsing faces...");

  mapFile.seekg(m_BspHeader.Lumps[Faces].iOffset, ios::beg);
  mapFile.read((char*) m_pFaces, m_BspHeader.Lumps[Faces].iLength);//sizeof( Q3BspFace_t ) * m_iNumFaces);

  /*for (int i = 0; i < m_iNumFaces; i++)			
  DEBUG_OUTPUT("face: " << i << ", index of first vertex " << m_pFaces[i].vertex );
  */
  mapFile.seekg(m_BspHeader.Lumps[Vertices].iOffset);
  mapFile.read((char*) m_pVertices, m_BspHeader.Lumps[Vertices].iLength);

  /*DEBUG_OUTPUT("num vertices: " << m_iNumVertices);
  for (int i=0; i<m_iNumVertices; i++)
  DEBUG_OUTPUT(i << ":" << m_pVertices[i].position[0]
  << ", " << m_pVertices[i].position[1]
  << ", " << m_pVertices[i].position[2]);*/

  mapFile.seekg(m_BspHeader.Lumps[MeshVerts].iOffset);
  mapFile.read((char*) m_pMeshVerts, m_BspHeader.Lumps[MeshVerts].iLength);

  /*DEBUG_OUTPUT("num meshverts: " << m_iNumMeshVerts);
  for (int i=0; i<m_iNumMeshVerts; i++)
  DEBUG_OUTPUT(i << ":" << m_pMeshVerts[i]);*/

  mapFile.seekg(m_BspHeader.Lumps[Leafs].iOffset);
  mapFile.read((char*) m_pLeafs, m_BspHeader.Lumps[Leafs].iLength);

  mapFile.seekg(m_BspHeader.Lumps[LeafFaces].iOffset);
  mapFile.read((char*) m_pLeafFaces, m_BspHeader.Lumps[LeafFaces].iLength);

  mapFile.seekg(m_BspHeader.Lumps[LeafBrushes].iOffset);
  mapFile.read((char*) m_pLeafBrushes, m_BspHeader.Lumps[LeafBrushes].iLength);

  mapFile.seekg(m_BspHeader.Lumps[Textures].iOffset);
  mapFile.read((char*) m_pTextures, m_BspHeader.Lumps[Textures].iLength);

  for (int i=0; i<m_iNumTexs; i++)
  {
    cout << m_pTextures[i].name << "\n";
  }

  mapFile.seekg(m_BspHeader.Lumps[Planes].iOffset);
  mapFile.read((char*) m_pPlanes, m_BspHeader.Lumps[Planes].iLength);		

  DEBUG_OUTPUT("number of nodes: " << m_iNumNodes);
  DEBUG_OUTPUT("parsing nodes...");

  mapFile.seekg(m_BspHeader.Lumps[Nodes].iOffset);
  mapFile.read((char*) m_pNodes, m_BspHeader.Lumps[Nodes].iLength);	

  /*for (int i = 0; i < m_iNumNodes; i++)
  DEBUG_OUTPUT("node: " << i << ", first child " << m_pNodes[i].children[0] << 
  ", second child " << m_pNodes[i].children[1]);
  */
  mapFile.seekg(m_BspHeader.Lumps[LightMaps].iOffset);
  mapFile.read((char*) m_pLightMaps, m_BspHeader.Lumps[LightMaps].iLength);

  mapFile.seekg(m_BspHeader.Lumps[Brushes].iOffset);
  mapFile.read((char*) m_pBrushes, m_BspHeader.Lumps[Brushes].iLength);

  mapFile.seekg(m_BspHeader.Lumps[BrushSides].iOffset);
  mapFile.read((char*) m_pBrushSides, m_BspHeader.Lumps[BrushSides].iLength);

  mapFile.seekg(m_BspHeader.Lumps[VisData].iOffset);
  mapFile.read((char*) m_VisData, sizeof(int)*2);

  int vec_size = m_VisData->n_vecs * m_VisData->sz_vecs;

  DEBUG_OUTPUT("n_vecs = " << m_VisData->n_vecs);
  DEBUG_OUTPUT("sz_vecs = " << m_VisData->sz_vecs);
  DEBUG_OUTPUT("vec_size = " << vec_size);

  m_VisData->vecs = new unsigned char[vec_size];

  mapFile.seekg(m_BspHeader.Lumps[VisData].iOffset + 2*sizeof(int));
  mapFile.read((char*) m_VisData->vecs, vec_size);	

  mapFile.close();		
}

void Q3Map::swizzleCoords(void)
{
  DEBUG_OUTPUT("swizziling data...");
  // vertices
  for (int i=0; i < m_iNumVertices; i++)
  {
    swizzleFloat3(m_pVertices[i].position);
    swizzleFloat3(m_pVertices[i].normal);
    //m_pVertices[i].texcoord[0][0] = 1.0f - m_pVertices[i].texcoord[0][0];		
  }

  // leafs
  for (int i=0; i < m_iNumLeafs; i++)
  {
    swizzleInt3(m_pLeafs[i].maxs);
    swizzleInt3(m_pLeafs[i].mins);
  }

  // faces, do lightmaps later...
  for (int i=0; i < m_iNumFaces; i++)
  {
    swizzleFloat3(m_pFaces[i].normal);
  }

  // planes
  for (int i=0; i < m_iNumPlanes; i++)
  {
    swizzleFloat3(m_pPlanes[i].normal);		
  }

  // nodes
  for (int i=0; i < m_iNumNodes; i++)
  {
    swizzleInt3(m_pNodes[i].maxs);
    swizzleInt3(m_pNodes[i].mins);
  }	
}

void Q3Map::swizzleFloat3(float t[3])
{	
  float temp;
  temp = t[1];
  t[1] = t[2];
  t[2] = -temp;
}

void Q3Map::swizzleInt3(int t[3])
{	
  int temp;
  temp = t[1];
  t[1] = t[2];
  t[2] = -temp;
}

void Q3Map::initMap()
{
  parseMap();

  swizzleCoords();

  mVisibleFaces = new int[m_iNumFaces];
}

Q3BspPatch *Q3Map::handlePatch(int faceIndex)
{
  Q3BspPatch *q3patch;
  q3patch = new Q3BspPatch;

  int patch_size_x = (m_pFaces[faceIndex].size[0] - 1) / 2;
  int patch_size_y = (m_pFaces[faceIndex].size[1] - 1) / 2;
  int num_bezier_patches = patch_size_y * patch_size_x;

  q3patch->size = num_bezier_patches;
  q3patch->bezier = new Bezier[q3patch->size];

  int patchIndex =  0;
  int ii, n, j, nn;
  for (ii = 0, n = 0; n < patch_size_x; n++, ii = 2*n)
  {				
    for (j=0, nn=0; nn < patch_size_y; nn++, j = 2*nn)
    {
      int index = 0;
      for (int ctr = 0; ctr < 3; ctr++)
      { 
        int pos = ctr * m_pFaces[faceIndex].size[0];

        q3patch->bezier[patchIndex].mControls[index++] = 
          BspVertex(
          // position
          m_pVertices[m_pFaces[faceIndex].vertex + 
          ii + 
          m_pFaces[faceIndex].size[0] * j + 
          pos].position,
          // texture coordinates
          m_pVertices[m_pFaces[faceIndex].vertex + 
          ii + 
          m_pFaces[faceIndex].size[0] * j +
          pos].texcoord,
          // normal
          m_pVertices[m_pFaces[faceIndex].vertex +
          ii +
          m_pFaces[faceIndex].size[0] * j +
          pos].normal);

        q3patch->bezier[patchIndex].mControls[index++] = BspVertex(m_pVertices[m_pFaces[faceIndex].vertex + ii + m_pFaces[faceIndex].size[0] * j + pos + 1].position,
          m_pVertices[m_pFaces[faceIndex].vertex + ii + m_pFaces[faceIndex].size[0] * j + pos + 1].texcoord,
          m_pVertices[m_pFaces[faceIndex].vertex + ii + m_pFaces[faceIndex].size[0] * j + pos + 1].normal);
        q3patch->bezier[patchIndex].mControls[index++] = BspVertex(m_pVertices[m_pFaces[faceIndex].vertex + ii + m_pFaces[faceIndex].size[0] * j + pos + 2].position,
          m_pVertices[m_pFaces[faceIndex].vertex + ii + m_pFaces[faceIndex].size[0] * j + pos + 2].texcoord,
          m_pVertices[m_pFaces[faceIndex].vertex + ii + m_pFaces[faceIndex].size[0] * j + pos + 2].normal);						
      }
      //q3patch->bezier[patchIndex].faceIndex = face;
      q3patch->bezier[patchIndex].tessellate(5);
      patchIndex++;
    }
  }

  return q3patch;
}

int Q3Map::findVisibleFaces(const D3DXVECTOR3 *camPos, int *facesToRender)
{
  int leaf;
  int visCluster;	

  leaf = findLeaf(camPos);	

  visCluster = m_pLeafs[leaf].cluster;

  memset(mVisibleFaces, 0, sizeof(int) * m_iNumFaces);	

  int faceindex;
  int renderindex=0;
  for (int i=0; i < m_iNumLeafs; i++)
  {
    if (isClusterVisible(visCluster, m_pLeafs[i].cluster))
    {									
      bool vis = mViewFrustum->checkIfBoxInside(m_pLeafs[i].mins, m_pLeafs[i].maxs);						
      if (vis)
      {
        for (int k=0; k < m_pLeafs[i].n_leaffaces; k++)
        {					
          faceindex =	m_pLeafFaces[m_pLeafs[i].leafface + k];				
          if (mVisibleFaces[faceindex] == 0)
          {
            mVisibleFaces[faceindex] = 1;						
            facesToRender[renderindex++] = faceindex;
          }
        }
      }			
    }
  }

  facesToRender[renderindex] = -1;	

  return renderindex;
}

int Q3Map::findLeaf(const D3DXVECTOR3 *camPos) const
{
  int index = 0;

  while (index >= 0)
  {
    const Q3BspNode *node = &m_pNodes[index];
    const Q3BspPlane *plane = &m_pPlanes[node->plane];

    // distance from point to plane
    D3DXVECTOR3 normal = D3DXVECTOR3(plane->normal);		 
    const float distance = D3DXVec3Dot(&normal,camPos) - plane->dist;

    if(distance >= 0)
      index = node->children[0];
    else
      index = node->children[1];
  }

  return -index - 1;
}

bool Q3Map::isClusterVisible(int visCluster, int testCluster) const
{
  if (m_VisData == NULL)
    return true;

  if ((m_VisData->vecs == NULL) || (visCluster < 0)) 	
    return true;    

  int i = (visCluster * m_VisData->sz_vecs) + (testCluster >> 3);
  unsigned char visSet = m_VisData->vecs[i];

  return (visSet & (1 << (testCluster & 7))) != 0;
}

Q3BspFace_t *Q3Map::getFaces(void)
{
  return m_pFaces;
}

void Q3Map::setViewFrustum(ViewFrustum *viewFrustum)
{
  mViewFrustum = viewFrustum;
}
