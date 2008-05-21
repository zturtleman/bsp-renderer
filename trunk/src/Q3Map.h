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

// Q3Map.h -- handles the map data

#ifndef _Q3MAP_H
#define _Q3MAP_H

#include "ViewFrustum.h"
#include "Bezier.h"

#include <string>
#include <vector>
#include <d3dx9.h>

using namespace std;

// the lump idexes
const int Textures = 1;
const int Planes = 2;
const int Nodes = 3;
const int Leafs = 4;
const int LeafFaces = 5;
const int LeafBrushes = 6;
const int Brushes = 8;
const int BrushSides = 9;
const int Vertices = 10;
const int MeshVerts = 11;
const int Faces = 13;
const int LightMaps = 14;
const int VisData = 16;

enum {POLYGON = 1, PATCH, MESH, BILLBOARD};

typedef struct{
	int iOffset;
	int iLength;
} direntry_t;

typedef struct {
	char name[64];
	int flags;
	int contents;
} Q3BspTexture;

typedef struct{
	unsigned char magic[4];
	int version;
	direntry_t Lumps[17];
} Q3BspHeader_t;

typedef struct{
	int texture;
	int effect;
	int type;
	int vertex;
	int n_vertexes;
	int meshvert;
	int n_meshverts;
	int lm_index;
	int lm_start[2];
	int lm_size[2];
	float lm_origin[3];
	float lm_vecs[2][3];
	float normal[3];
	int size[2];
} Q3BspFace_t;

typedef struct{
	float position[3];
	float texcoord[2][2];
	float normal[3];
	unsigned char color[4];
} Q3BspVertex;

typedef struct {
	int cluster;
	int area;
	int mins[3];
	int maxs[3];
	int leafface;
	int n_leaffaces;
	int leafbrush;
	int n_leafbrushes;
} Q3BspLeaf;

typedef struct {
	float normal[3];
	float dist;
} Q3BspPlane;

typedef struct {
	int plane;
	int children[2];
	int mins[3];
	int maxs[3];
} Q3BspNode;

typedef struct {
	int brushside;
	int n_brushsides;
	int texture;
} Q3BspBrush;

typedef struct {
	int plane;
	int texture;
} Q3BspBrushSide;

typedef struct {
	unsigned char lightmap[128][128][3];
} Q3BspLightMap;

typedef struct {
	int n_vecs;
	int sz_vecs;
	unsigned char *vecs;
} Q3BspVisData;
	
typedef struct {
	int size;
	Bezier *bezier;
} Q3BspPatch;

class Q3Map
{
public:
	Q3Map();
	~Q3Map();	

	void loadMap(string cfgFile);	
	int findVisibleFaces(const D3DXVECTOR3 *camPos, int *facesToRender);
	Q3BspFace_t *getFaces(void);
	Q3BspPatch *handlePatch(int faceIndex);
	void setViewFrustum(ViewFrustum *viewFrustum);

private:

	void parseMap(void);
	void initMap();	
	//void handlePatches(void);

	void swizzleCoords(void);
	void swizzleFloat3(float t[3]);
	void swizzleInt3(int t[3]);	

	int findLeaf(const D3DXVECTOR3 *camPos) const;
	bool isClusterVisible(int visCluster, int testCluster) const;	

	int *mVisibleFaces;
	ViewFrustum *mViewFrustum;

	std::vector<Q3BspVisData> *patches;

public:
	// map data
	vector<string> pk3Files;
	string mapName;

	Q3BspHeader_t m_BspHeader;

	char *m_pEntities;

	int m_iNumTexs;
	Q3BspTexture *m_pTextures;

	int m_iNumFaces;
	Q3BspFace_t *m_pFaces; 

	int m_iNumVertices;
	Q3BspVertex *m_pVertices;

	int m_iNumMeshVerts;	
	int *m_pMeshVerts;

	int m_iNumLeafs;
	Q3BspLeaf *m_pLeafs;

	int m_iNumLeafFaces;
	int *m_pLeafFaces;

	int m_iNumPlanes;
	Q3BspPlane *m_pPlanes;

	int m_iNumNodes;
	Q3BspNode *m_pNodes;

	int m_iNumLeafBrushes;
	int *m_pLeafBrushes;

	int m_iNumBrushes;
	Q3BspBrush *m_pBrushes;

	int m_iNumBrushSides;
	Q3BspBrushSide *m_pBrushSides;

	int m_iNumLightMaps;
	Q3BspLightMap *m_pLightMaps;

	Q3BspVisData *m_VisData;

};

#endif /* _Q3MAP_H */