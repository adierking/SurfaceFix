#pragma once
#include "stdafx.h"

extern int CreateSurfaceHook(int id, unsigned int width, unsigned int height);
extern LPDIRECT3DSURFACE8 SetRenderTargetHook(int id);
extern void FreeSurfaceHook(int id);
extern void BeforeDeviceResetHook();
extern void AfterDeviceResetHook();
extern int D3DSwitchOverride(int switchToD3d);
extern void SetViewportForSurfaceHook();
extern void ResetViewportHook();

extern BYTE* tempString;