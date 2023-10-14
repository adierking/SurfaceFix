#pragma once
#include "stdafx.h"

extern LPDIRECT3D8 d3d;
extern LPDIRECT3DDEVICE8 d3dDevice;
extern LPDIRECT3DSURFACE8 oldDepthBuffer;

extern int* gmCurrentSurface;
extern BYTE* gmBinFileStatus;
extern BYTE* gmBinFileData;
extern BYTE** gmSurfaceData;
extern DWORD gmSurfaceExists;
extern DWORD gmGetSurfacePtr;
extern DWORD* gmRenderWidth;
extern DWORD* gmRenderHeight;
extern D3DCAPS8* gmDevCaps;

extern int DetectGMVersion();
extern void InstallHooks();