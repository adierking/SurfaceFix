// hooks.cpp - The runner hook stuff is in here.

#include "stdafx.h"
#include "hooks.h"
#include "SurfaceFix.h"

// Branching defines for the InsertBranch() function
// These are actually just x86 opcodes.
#define BRANCH_CALL 0xE8
#define BRANCH_JUMP 0xE9

// Version check info
// The address 0x500000 isn't of any significance to the runner; it's just an address I chose
#define GM_VERSION 0x500000
#define GM7_ID 0x589A24			// Game Maker 7
#define GM7_IP_ID 0xC43890C		// Game Maker 7 in Instant Play
#define GM8_ID 0xE982754F		// Game Maker 8
#define GM8_IP_ID 0x006A5000	// Game Maker 8 in Instant Play

////////////////////////////////////////
// GM7 and GM7/Instant Play addresses //
////////////////////////////////////////

// Most of the Instant Play addresses seem to be offset from the originals by 0xB4.
// Other addresses seem to be offset by 0x140, and some even remain unmoved.

#define GM7_SET_STRING 0x4051A0
#define GM7_ALLOCATE_STRING 0x40506C
#define GM7_CLEAR_STRING 0x404FA8

#define GM7_BINFILE_STATUS_ARRAY 0x5F5544
#define GM7_BINFILE_DATA_TABLE 0x5F2A78

#define GM7_SURFACE_DATA 0x5AF14C
#define GM7_CURRENT_SURFACE 0x587A5C
#define GM7_SURFACE_EXISTS 0x4A297C
	#define GM7_IP_SURFACE_EXISTS 0x4A2A30
#define GM7_GET_SURFACE_PTR 0x4A4D2C
	#define GM7_IP_GET_SURFACE_PTR 0x4A4DE0

#define GM7_VIEW_INFO 0x589D70
#define GM7_VIEW_CURRENT 0x589B98
#define GM7_GET_VIEW_DATA 0x4AEC3C
	#define GM7_IP_GET_VIEW_DATA 0x4AECF8

#define GM7_SET_PROJECTION 0x4A4028
	#define GM7_IP_SET_PROJECTION 0x4A40DC
#define GM7_SET_VIEWPORT 0x4A3FE4
	#define GM7_IP_SET_VIEWPORT 0x4A4098
#define GM7_GET_RENDER_SIZE 0x4A3FD4
	#define GM7_IP_GET_RENDER_SIZE 0x4A4088

#define GM7_SET_D3D_STATES 0x4A3914
	#define GM7_IP_SET_D3D_STATES 0x4A39C8
#define GM7_BEGIN_SCENE 0x4A42B4
	#define GM7_IP_BEGIN_SCENE 0x4A4368
#define GM7_WAIT_VSYNC 0x49E458
	#define GM7_IP_WAIT_VSYNC 0x49E50C

#define GM7_D3D_INTERFACE 0x587A60
#define GM7_D3D_DEVICE 0x587A64
#define GM7_D3D_DEVICE_PTR 0x589B08
#define GM7_SCREEN_TARGET 0x587A58
#define GM7_RENDER_WIDTH 0x587A68
#define GM7_RENDER_HEIGHT 0x587A6C
#define GM7_PERSPECTIVE_PROJ 0x587A84
#define GM7_D3D_MODE 0x589BAC
#define GM7_DEVICE_CAPS 0x5AF150

#define GM7_RESET_RENDER_TARGET 0x4A2BA8
	#define GM7_IP_RESET_RENDER_TARGET 0x4A2C5C
#define GM7_DESTROY_SURFACES 0x4A29E8
	#define GM7_IP_DESTROY_SURFACES 0x4A2A9C
#define GM7_RESET_DEVICE 0x4A3E20
	#define GM7_IP_RESET_DEVICE 0x4A3ED4

#define GM7_CREATESURFACE_HOOK 0x4A2957
#define GM7_IP_CREATESURFACE_HOOK 0x4A2A0B
	#define GM7_CREATESURFACE_HOOK_SUCCESS 0x4A295E
		#define GM7_IP_CREATESURFACE_HOOK_SUCCESS 0x4A2A12
	#define GM7_CREATESURFACE_HOOK_ERROR 0x4A2976
		#define GM7_IP_CREATESURFACE_HOOK_ERROR 0x4A2A2A

#define GM7_SETRENDERTARGET_HOOK 0x4A2B12
#define GM7_IP_SETRENDERTARGET_HOOK 0x4A2BC6
	#define GM7_SETRENDERTARGET_HOOK_SUCCESS 0x4A2B17
		#define GM7_IP_SETRENDERTARGET_HOOK_SUCCESS 0x4A2BCB

#define GM7_FREESURFACE_HOOK 0x4A29CD
	#define GM7_IP_FREESURFACE_HOOK 0x4A2A81

#define GM7_RESETRENDERTARGET_HOOK 0x4A2BC7
#define GM7_IP_RESETRENDERTARGET_HOOK 0x4A2C7B
	#define GM7_RESETRENDERTARGET_HOOK_SUCCESS 0x4A2BCC
		#define GM7_IP_RESETRENDERTARGET_HOOK_SUCCESS 0x4A2C80

#define GM7_SETRENDERTARGET_PROJECTION_HOOK 0x4A2B4A
#define GM7_IP_SETRENDERTARGET_PROJECTION_HOOK 0x4A2BFE
	#define GM7_SETRENDERTARGET_PROJECTION_HOOK_SUCCESS 0x4A2B7B
		#define GM7_IP_SETRENDERTARGET_PROJECTION_HOOK_SUCCESS 0x4A2C2F
	#define GM7_SETRENDERTARGET_PROJECTION_HOOK_NOVIEWS 0x4A2B50
		#define GM7_IP_SETRENDERTARGET_PROJECTION_HOOK_NOVIEWS 0x4A2C04

#define GM7_RESETRENDERTARGET_VIEWPORT_HOOK 0x4A2BE9
#define GM7_IP_RESETRENDERTARGET_VIEWPORT_HOOK 0x4A2C9D
	#define GM7_RESETRENDERTARGET_VIEWPORT_HOOK_SUCCESS 0x4A2C27
		#define GM7_IP_RESETRENDERTARGET_VIEWPORT_HOOK_SUCCESS 0x4A2CDB
	#define GM7_RESETRENDERTARGET_VIEWPORT_HOOK_NOVIEWS 0x4A2BEE
		#define GM7_IP_RESETRENDERTARGET_VIEWPORT_HOOK_NOVIEWS 0x4A2CA2

#define GM7_BEFOREDEVICERESET_HOOK 0x4A3EAB
	#define GM7_IP_BEFOREDEVICERESET_HOOK 0x4A3F5F
#define GM7_AFTERDEVICERESET_HOOK 0x4A3F14
	#define GM7_IP_AFTERDEVICERESET_HOOK 0x4A3FC8

#define GM7_TRANSITION_BEGINSCENE_HOOK 0x523039
	#define GM7_IP_TRANSITION_BEGINSCENE_HOOK 0x523179
#define GM7_TRANSITION_DONE_HOOK 0x523083
	#define GM7_IP_TRANSITION_DONE_HOOK 0x5231C3

#define GM7_D3DSWITCH_OVERRIDE_1 0x4A03CC
	#define GM7_IP_D3DSWITCH_OVERRIDE_1 0x4A0480
#define GM7_D3DSWITCH_OVERRIDE_2 0x4A03E6
	#define GM7_IP_D3DSWITCH_OVERRIDE_2 0x4A049A

#define GM7_DRAWCLEAR_HOOK 0x49E815
	#define GM7_IP_DRAWCLEAR_HOOK 0x49E8C9
#define GM7_DRAWCLEARALPHA_HOOK 0x49E84A
	#define GM7_IP_DRAWCLEARALPHA_HOOK 0x49E8FE

#define GM7_DLLHOOK 0x51B11D
	#define GM7_IP_DLLHOOK 0x51B25D

#define GM7_PRESENTSCENE_HOOK_1 0x4A437A
#define GM7_IP_PRESENTSCENE_HOOK_1 0x4A442E
	#define GM7_PRESENTSCENE_HOOK_1_SUCCESS 0x4A437F
	#define GM7_IP_PRESENTSCENE_HOOK_1_SUCCESS 0x4A4433
#define GM7_PRESENTSCENE_HOOK_2 0x4A4425
#define GM7_IP_PRESENTSCENE_HOOK_2 0x4A44D9
	#define GM7_PRESENTSCENE_HOOK_2_SUCCESS 0x4A442A
	#define GM7_IP_PRESENTSCENE_HOOK_2_SUCCESS 0x4A44DE

#define GM7_GAMELOOP_HOOK 0x54074F
#define GM7_IP_GAMELOOP_HOOK 0x54088F
	#define GM7_GAMELOOP_HOOK_SUCCESS 0x53FDF4
		#define GM7_IP_GAMELOOP_HOOK_SUCCESS 0x53FF34
	#define GM7_GAMELOOP_HOOK_FAILURE 0x5409D3
		#define GM7_IP_GAMELOOP_HOOK_FAILURE 0x540B13

////////////////////////////////////////
// GM8 and GM8/Instant Play addresses //
////////////////////////////////////////
#define GM8_SET_STRING 0x405B2C

#define GM8_ALLOCATE_STRING 0x4059F8
#define GM8_CLEAR_STRING 0x405934

#define GM8_BINFILE_STATUS_ARRAY 0x707CDC
#define GM8_BINFILE_DATA_TABLE 0x705210

#define GM8_SURFACE_DATA 0x6C7240
#define GM8_CURRENT_SURFACE 0x58D380
#define GM8_SURFACE_EXISTS 0x4A0D30
	#define GM8_IP_SURFACE_EXISTS 0x4A0D54
#define GM8_GET_SURFACE_PTR 0x4A35BC
	#define GM8_IP_GET_SURFACE_PTR 0x4A35E0

#define GM8_VIEW_INFO 0x58FAC4
#define GM8_VIEW_CURRENT 0x58F908
#define GM8_GET_VIEW_DATA 0x4AD5D4
	#define GM8_IP_GET_VIEW_DATA 0x4AD5FC

#define GM8_SET_PROJECTION 0x4A2440
	#define GM8_IP_SET_PROJECTION 0x4A2464
#define GM8_SET_VIEWPORT 0x4A23FC
	#define GM8_IP_SET_VIEWPORT 0x4A2420
#define GM8_GET_RENDER_SIZE 0x4A23EC
	#define GM8_IP_GET_RENDER_SIZE 0x4A2410

#define GM8_SET_D3D_STATES 0x4A1CFC
	#define GM8_IP_SET_D3D_STATES 0x4A1D20
#define GM8_BEGIN_SCENE 0x4A26E8
	#define GM8_IP_BEGIN_SCENE 0x4A270C
#define GM8_WAIT_VSYNC 0x49C6C4
	#define GM8_IP_WAIT_VSYNC 0x49C6E8

#define GM8_RESET_RENDER_TARGET 0x4A0F6C
	#define GM8_IP_RESET_RENDER_TARGET 0x4A0F90
#define GM8_DESTROY_SURFACES 0x4A0D9C
	#define GM8_IP_DESTROY_SURFACES 0x4A0DC0
#define GM8_RESET_DEVICE 0x4A2228
	#define GM8_IP_RESET_DEVICE 0x4A224C

#define GM8_D3D_INTERFACE 0x58D384
#define GM8_D3D_DEVICE 0x58D388
#define GM8_D3D_DEVICE_PTR 0x58FBE4
#define GM8_SCREEN_TARGET 0x58D37C
#define GM8_RENDER_WIDTH 0x58D38C
#define GM8_RENDER_HEIGHT 0x58D390
#define GM8_PERSPECTIVE_PROJ 0x58D3A8
#define GM8_D3D_MODE 0x58F914
#define GM8_DEVICE_CAPS 0x6C7244

#define GM8_CREATESURFACE_HOOK 0x4A0D0B
#define GM8_IP_CREATESURFACE_HOOK 0x4A0D2F
	#define GM8_CREATESURFACE_HOOK_SUCCESS 0x4A0D12
		#define GM8_IP_CREATESURFACE_HOOK_SUCCESS 0x4A0D36
	#define GM8_CREATESURFACE_HOOK_ERROR 0x4A0D2A
		#define GM8_IP_CREATESURFACE_HOOK_ERROR 0x4A0D4E

#define GM8_SETRENDERTARGET_HOOK 0x4A0ECD
#define GM8_IP_SETRENDERTARGET_HOOK 0x4A0EF1
	#define GM8_SETRENDERTARGET_HOOK_SUCCESS 0x4A0ED2
		#define GM8_IP_SETRENDERTARGET_HOOK_SUCCESS 0x4A0EF6

#define GM8_FREESURFACE_HOOK 0x4A0D81
	#define GM8_IP_FREESURFACE_HOOK 0x4A0DA5

#define GM8_RESETRENDERTARGET_HOOK 0x4A0F8B
#define GM8_IP_RESETRENDERTARGET_HOOK 0x4A0FAF
	#define GM8_RESETRENDERTARGET_HOOK_SUCCESS 0x4A0F90
		#define GM8_IP_RESETRENDERTARGET_HOOK_SUCCESS 0x4A0FB4

#define GM8_SETRENDERTARGET_PROJECTION_HOOK 0x4A0F0C
#define GM8_IP_SETRENDERTARGET_PROJECTION_HOOK 0x4A0F30
	#define GM8_SETRENDERTARGET_PROJECTION_HOOK_SUCCESS 0x4A0F3D
		#define GM8_IP_SETRENDERTARGET_PROJECTION_HOOK_SUCCESS 0x4A0F61
	#define GM8_SETRENDERTARGET_PROJECTION_HOOK_NOVIEWS 0x4A0F12
		#define GM8_IP_SETRENDERTARGET_PROJECTION_HOOK_NOVIEWS 0x4A0F36

#define GM8_RESETRENDERTARGET_VIEWPORT_HOOK 0x4A0FB4
#define GM8_IP_RESETRENDERTARGET_VIEWPORT_HOOK 0x4A0FD8
	#define GM8_RESETRENDERTARGET_VIEWPORT_HOOK_SUCCESS 0x4A0FF2
		#define GM8_IP_RESETRENDERTARGET_VIEWPORT_HOOK_SUCCESS 0x4A1016
	#define GM8_RESETRENDERTARGET_VIEWPORT_HOOK_NOVIEWS 0x4A0FB9
		#define GM8_IP_RESETRENDERTARGET_VIEWPORT_HOOK_NOVIEWS 0x4A0FDD

#define GM8_BEFOREDEVICERESET_HOOK 0x4A22BC
	#define GM8_IP_BEFOREDEVICERESET_HOOK 0x4A22E0
#define GM8_AFTERDEVICERESET_HOOK 0x4A2333
	#define GM8_IP_AFTERDEVICERESET_HOOK 0x4A2357

#define GM8_TRANSITION_BEGINSCENE_HOOK 0x51EEA1
	#define GM8_IP_TRANSITION_BEGINSCENE_HOOK 0x51EF4D
#define GM8_TRANSITION_DONE_HOOK 0x51EEEB
	#define GM8_IP_TRANSITION_DONE_HOOK 0x51EF97

#define GM8_D3DSWITCH_OVERRIDE_1 0x49E700
	#define GM8_IP_D3DSWITCH_OVERRIDE_1 0x49E724
#define GM8_D3DSWITCH_OVERRIDE_2 0x49E71A
	#define GM8_IP_D3DSWITCH_OVERRIDE_2 0x49E73E

#define GM8_DRAWCLEAR_HOOK 0x49CB01
	#define GM8_IP_DRAWCLEAR_HOOK 0x49CB25
#define GM8_DRAWCLEARALPHA_HOOK 0x49CB36
	#define GM8_IP_DRAWCLEARALPHA_HOOK 0x49CB5A

#define GM8_DLLHOOK 0x517946
	#define GM8_IP_DLLHOOK 0x5179F2

#define GM8_PRESENTSCENE_HOOK_1 0x4A27AE
#define GM8_IP_PRESENTSCENE_HOOK_1 0x4A27D2
	#define GM8_PRESENTSCENE_HOOK_1_SUCCESS 0x4A27BA
		#define GM8_IP_PRESENTSCENE_HOOK_1_SUCCESS 0x4A27DE
#define GM8_PRESENTSCENE_HOOK_2 0x4A2861
#define GM8_IP_PRESENTSCENE_HOOK_2 0x4A2885
	#define GM8_PRESENTSCENE_HOOK_2_SUCCESS 0x4A2866
		#define GM8_IP_PRESENTSCENE_HOOK_2_SUCCESS 0x4A288A

#define GM8_GAMELOOP_HOOK 0x53E577
#define GM8_IP_GAMELOOP_HOOK 0x53E623
	#define GM8_GAMELOOP_HOOK_SUCCESS 0x53DAA4
		#define GM8_IP_GAMELOOP_HOOK_SUCCESS 0x53DB50
	#define GM8_GAMELOOP_HOOK_FAILURE 0x53E808
		#define GM8_IP_GAMELOOP_HOOK_FAILURE 0x53E8B4

/////////////////////////////////////////////////////////////////////////////
// These variables store the above addresses, depending on the GM version. //
/////////////////////////////////////////////////////////////////////////////
DWORD* gmVersionID = (DWORD*)GM_VERSION;

DWORD gmSetString = GM7_SET_STRING;
DWORD gmAllocString = GM7_ALLOCATE_STRING;
DWORD gmClearString = GM7_CLEAR_STRING;

BYTE* gmBinFileStatus = (BYTE*)GM7_BINFILE_STATUS_ARRAY;
BYTE* gmBinFileData = (BYTE*)GM7_BINFILE_DATA_TABLE;

BYTE** gmSurfaceData = (BYTE**)GM7_SURFACE_DATA;
int* gmCurrentSurface = (int*)GM7_CURRENT_SURFACE;
DWORD gmSurfaceExists = GM7_SURFACE_EXISTS;
DWORD gmGetSurfacePtr = GM7_GET_SURFACE_PTR;

BYTE* gmViewInfo = (BYTE*)GM7_VIEW_INFO;
DWORD** gmCurrentView = (DWORD**)GM7_VIEW_CURRENT;
DWORD gmGetViewData = GM7_GET_VIEW_DATA;

DWORD gmSetProjection = GM7_SET_PROJECTION;
DWORD gmSetViewport = GM7_SET_VIEWPORT;
DWORD gmGetRenderSize = GM7_GET_RENDER_SIZE;

DWORD gmSetD3DStates = GM7_SET_D3D_STATES;
DWORD gmBeginScene = GM7_BEGIN_SCENE;
DWORD gmWaitVSync = GM7_WAIT_VSYNC;

DWORD gmResetTarget = GM7_RESET_RENDER_TARGET;
DWORD gmDestroySurfaces = GM7_DESTROY_SURFACES;
DWORD gmResetDevice = GM7_RESET_DEVICE;

LPDIRECT3D8* gmD3DInterface = (LPDIRECT3D8*)GM7_D3D_INTERFACE;
LPDIRECT3DDEVICE8* gmD3DDevice = (LPDIRECT3DDEVICE8*)GM7_D3D_DEVICE;
LPDIRECT3DDEVICE8** gmD3DDevicePtr = (LPDIRECT3DDEVICE8**)GM7_D3D_DEVICE_PTR;
LPDIRECT3DSURFACE8* gmScreenSurface = (LPDIRECT3DSURFACE8*)GM7_SCREEN_TARGET;
DWORD* gmRenderWidth = (DWORD*)GM7_RENDER_WIDTH;
DWORD* gmRenderHeight = (DWORD*)GM7_RENDER_HEIGHT;
BYTE* gmPerspProj = (BYTE*)GM7_PERSPECTIVE_PROJ;
BYTE* gmD3DMode = (BYTE*)GM7_D3D_MODE;
D3DCAPS8* gmDevCaps = (D3DCAPS8*)GM7_DEVICE_CAPS;

DWORD gmCreateSurfaceHook = GM7_CREATESURFACE_HOOK;
DWORD gmCreateSurfaceHook_Success = GM7_CREATESURFACE_HOOK_SUCCESS;
DWORD gmCreateSurfaceHook_Error = GM7_CREATESURFACE_HOOK_ERROR;

DWORD gmSetRenderTargetHook = GM7_SETRENDERTARGET_HOOK;
DWORD gmSetRenderTargetHook_Success = GM7_SETRENDERTARGET_HOOK_SUCCESS;

DWORD gmFreeSurfaceHook = GM7_FREESURFACE_HOOK;

DWORD gmResetRenderTargetHook = GM7_RESETRENDERTARGET_HOOK;
DWORD gmResetRenderTargetHook_Success = GM7_RESETRENDERTARGET_HOOK_SUCCESS;

DWORD gmSetRenderTargetProjectionHook = GM7_SETRENDERTARGET_PROJECTION_HOOK;
DWORD gmSetRenderTargetProjectionHook_Success = GM7_SETRENDERTARGET_PROJECTION_HOOK_SUCCESS;
DWORD gmSetRenderTargetProjectionHook_NoViews = GM7_SETRENDERTARGET_PROJECTION_HOOK_NOVIEWS;

DWORD gmResetRenderTargetViewportHook = GM7_RESETRENDERTARGET_VIEWPORT_HOOK;
DWORD gmResetRenderTargetViewportHook_Success = GM7_RESETRENDERTARGET_VIEWPORT_HOOK_SUCCESS;
DWORD gmResetRenderTargetViewportHook_NoViews = GM7_RESETRENDERTARGET_VIEWPORT_HOOK_NOVIEWS;

DWORD gmBeforeDeviceResetHook = GM7_BEFOREDEVICERESET_HOOK;
DWORD gmAfterDeviceResetHook = GM7_AFTERDEVICERESET_HOOK;

DWORD gmTransitionBeginSceneHook = GM7_TRANSITION_BEGINSCENE_HOOK;
DWORD gmTransitionDoneHook = GM7_TRANSITION_DONE_HOOK;

DWORD gmD3DSwitchOverride1 = GM7_D3DSWITCH_OVERRIDE_1;
DWORD gmD3DSwitchOverride2 = GM7_D3DSWITCH_OVERRIDE_2;

DWORD gmDrawClearHook = GM7_DRAWCLEAR_HOOK;
DWORD gmDrawClearAlphaHook = GM7_DRAWCLEARALPHA_HOOK;

DWORD gmDLLHook = GM7_DLLHOOK;

DWORD gmPresentSceneHook1 = GM7_PRESENTSCENE_HOOK_1;
DWORD gmPresentSceneHook1_Success = GM7_PRESENTSCENE_HOOK_1_SUCCESS;
DWORD gmPresentSceneHook2 = GM7_PRESENTSCENE_HOOK_2;
DWORD gmPresentSceneHook2_Success = GM7_PRESENTSCENE_HOOK_2_SUCCESS;

DWORD gmGameLoopHook = GM7_GAMELOOP_HOOK;
DWORD gmGameLoopHook_Success = GM7_GAMELOOP_HOOK_SUCCESS;
DWORD gmGameLoopHook_Failure = GM7_GAMELOOP_HOOK_FAILURE;

// D3D pointers
LPDIRECT3D8 d3d;
LPDIRECT3DDEVICE8 d3dDevice;
LPDIRECT3DSURFACE8 oldDepthBuffer = 0;

// Global Variables
int perspProj = 0;
int deviceLost = 0;

// Inserts a branching instruction at the specified address, replacing the old instruction(s).
// The inserted instruction occupies 5 bytes (1 byte for the branch instruction + 4 bytes for the address).
void InsertBranch(BYTE branchType, void* writeAddr, void* destAddr)
{
	DWORD oldProtection;
	DWORD relAddr = (DWORD)destAddr;

	// Unprotect the memory area needed to insert the instruction
	VirtualProtect(writeAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtection);

	// Insert the branch instruction
	*(BYTE*)writeAddr = branchType;

	// Calculate and insert the relative offset of the address to branch to
	relAddr -= (DWORD)writeAddr + 5;
	*(DWORD*)((BYTE*)(writeAddr) + 1) = relAddr;

	// Flush the instruction cache
	FlushInstructionCache(GetCurrentProcess(), writeAddr, 5);
}

// The base code for the CreateSurface hook.
// This sets some stuff up and then calls a C++ hook function.
__declspec(naked) void CreateSurfaceHookBase()
{
	__asm
	{
		// This was the replaced instruction
		jl Error

		// Push the surface height (in [esp + 4]), the surface width (in ebp), and the ID (in ebx)
		// Then call the C++ hook function
		mov eax, [esp]
		push eax
		push ebp
		push ebx
		call CreateSurfaceHook
		add esp, 12

		// If the depth buffer could not be created, make the function fail
		test eax, eax
		jz Error

		// Jump to the spot after the replaced code
		mov edx, gmCreateSurfaceHook_Success
		jmp edx

Error:
		// Set eax to -1 and jump to the part of the code that handles an error
		or eax, 0xFFFFFFFF
		mov edx, gmCreateSurfaceHook_Error
		jmp edx
	}
}

// The base code for the SetRenderTarget hook.
// This first calls a C++ function to save viewport and projection information, and then calls another C++ function to get the depth buffer address
__declspec(naked) void SetRenderTargetHookBase()
{
	__asm
	{
		// edi contains the surface ID, so push it on, call the C++ function, and push the depth buffer address onto the stack
		push edi
		call SetRenderTargetHook
		pop edi
		push eax

		// Execute some of the replaced code
		mov eax, [ebp-04h]

		// Jump to the spot after the replaced code
		mov edx, gmSetRenderTargetHook_Success
		jmp edx
	}
}

// The base code for the FreeSurface hook.
// Again, this just calls some C++ code.
__declspec(naked) void FreeSurfaceHookBase()
{
	__asm
	{
		// Push ebx (the ID) and call the C++ function
		push ebx
		call FreeSurfaceHook
		pop ebx

		// Execute the replaced code and then return
		mov eax, gmSurfaceData
		mov eax, [eax]
		ret
	}
}

// The code for the hook that is called when the render target is reset.
// No C++ code is needed here - some stack manipulation just needs to be done.
__declspec(naked) void ResetRenderTargetHook()
{
	__asm
	{
		// Get the old depth buffer pointer and put it on the stack
		mov eax, oldDepthBuffer
		mov [esp], eax

		// Execute the replaced code
		mov eax, gmScreenSurface
		mov eax, [eax]

		// Jump to the spot after the replaced code
		mov edx, gmResetRenderTargetHook_Success
		jmp edx
	}
}

// The base code for the hook that is called before the device is reset.
// This calls a C++ hook function that is responsible for freeing the depth buffer pointer.
__declspec(naked) void BeforeDeviceResetHookBase()
{
	__asm
	{
		// Execute the replaced code
		mov edx, gmDestroySurfaces
		call edx

		// Call the C++ function
		jmp BeforeDeviceResetHook
	}
}

// The base code for the hook that is called after the device is reset.
// This calls a C++ hook function that retrieves the new depth buffer pointer.
__declspec(naked) void AfterDeviceResetHookBase()
{
	__asm
	{
		// Execute the replaced code
		mov eax, gmSetD3DStates
		call eax

		// Call the C++ hook function
		jmp AfterDeviceResetHook
	}
}

// The base code for the hook that overrides part of d3d_start() and d3d_end().
// It calls a C++ function that creates/destroys the depth buffer and does a few operations on the device.
// Then, it sets up all of the render states.
__declspec(naked) void D3DSwitchOverrideBase()
{
	__asm
	{
		// If we are drawing to a surface, reset the render target
		mov eax, gmCurrentSurface
		mov eax, [eax]
		cmp eax, 0xFFFFFFFF
		je RenderTargetReset
		mov edx, gmResetTarget
		call edx

RenderTargetReset:
		// Retrieve whether or not we are switching to 3D mode
		mov eax, gmD3DMode
		mov eax, [eax]
		mov al, byte ptr [eax]
		xor ah, ah
		push eax

		// Call the C++ function
		call D3DSwitchOverride
		add esp, 4

		// Test if it failed
		test eax, eax
		jz Failed

		// Call a runner function to reset render states
		mov edx, gmSetD3DStates
		call edx

		// Return
		mov eax, 1

Failed:
		ret
	}
}

// The code for the hook that is called when setting the projection for a surface.
// This overrides GM's code if views are enabled and takes views into account when setting the projection.
__declspec(naked) void SetProjectionForSurfaceHook()
{
	__asm
	{
		// Check if views are enabled, and if not, execute the replaced code and return
		mov eax, gmViewInfo
		mov eax, [eax]
		mov eax, [eax]
		cmp byte ptr [eax + 64], 0
		je ViewsDisabled

		// Get the current view's ID
		mov edx, gmCurrentView
		mov edx, [edx]
		mov edx, [edx]

		// Call a runner function to get a pointer to the view's data
		mov ecx, gmGetViewData
		call ecx

		// Push the view information onto the stack
		// View X
		fild dword ptr [eax + 8]
		add esp, -8
		fstp qword ptr [esp]
		wait

		// View Y
		fild dword ptr [eax + 12]
		add esp, -8
		fstp qword ptr [esp]
		wait

		// Surface Width
		mov edx, gmSurfaceData
		mov edx, [edx]
		fild dword ptr [edx + esi * 8 + 4]
		add esp, -8
		fstp qword ptr [esp]
		wait

		// Surface Height
		fild dword ptr [edx + esi * 8 + 8]
		add esp, -8
		fstp qword ptr [esp]
		wait

		// View Rotation
		push [eax + 44]
		push [eax + 40]

		// Call a runner function to set the projection
		mov ecx, gmSetProjection
		call ecx

		// Jump out of the hook and skip GM's faulty projection code
		mov edx, gmSetRenderTargetProjectionHook_Success
		jmp edx

ViewsDisabled:
		// Execute the replaced code
		push 0
		push 0
		push 0

		// Jump to the location after the hook
		mov edx, gmSetRenderTargetProjectionHook_NoViews
		jmp edx
	}
}

// The code for the hook that is called when resetting the viewport and projection back to the normal ones.
// If views are enabled, this takes them into account.
__declspec(naked) void ResetViewportHook()
{
	__asm
	{
		// Check if views are enabled, and if not, execute the replaced code and return
		push eax
		mov eax, gmViewInfo
		mov eax, [eax]
		mov eax, [eax]
		cmp byte ptr [eax + 64], 0
		je ViewsDisabled
		add esp, 4

		// Get the current view's ID
		mov edx, gmCurrentView
		mov edx, [edx]
		mov edx, [edx]

		// Call a runner function to get a pointer to the view's data
		mov ecx, gmGetViewData
		call ecx
		push ebx
		mov ebx, eax
		push edi

		// Set the viewport parameters
		// Viewport Height
		mov eax, [ebx + 36]
		push eax

		// Viewport X
		mov eax, [ebx + 24]

		// Viewport Y
		mov edx, [ebx + 28]

		// Viewport Width
		mov ecx, [ebx + 32]

		// Call a runner function to set the viewport
		mov edi, gmSetViewport
		call edi
		pop edi

		// Set the projection
		// View X
		fild dword ptr [ebx + 8]
		add esp, -8
		fstp qword ptr [esp]
		wait

		// View Y
		fild dword ptr [ebx + 12]
		add esp, -8
		fstp qword ptr [esp]
		wait

		// View Width
		fild dword ptr [ebx + 16]
		add esp, -8
		fstp qword ptr [esp]
		wait

		// View Height
		fild dword ptr [ebx + 20]
		add esp, -8
		fstp qword ptr [esp]
		wait

		// View Rotation
		push [ebx + 44]
		push [ebx + 40]

		// Call a runner function to set the projection
		mov ecx, gmSetProjection
		call ecx

		// Skip GM's code to reset the viewport and surface
		pop ebx
		mov edx, gmResetRenderTargetViewportHook_Success
		jmp edx

ViewsDisabled:
		// Execute the replaced code
		pop eax
		mov ecx, gmGetRenderSize
		call ecx

		// Jump out of the hook
		mov edx, gmResetRenderTargetViewportHook_NoViews
		jmp edx
	}
}

// The code for the hook that is called when a transition is about to be drawn.
// This temporarily switches to an orthographic projection so that drawing the surfaces works correctly in 3D mode.
// No C++ code needs to be called here - runner functions are used to set the view and projection.
__declspec(naked) void TransitionBeginSceneHook()
{
	__asm
	{
		// Call BeginScene (the replaced code)
		mov eax, gmBeginScene
		call eax

		// Save and then clear the "perspective projection" variable
		mov eax, gmPerspProj
		mov dl, byte ptr [eax]
		mov perspProj, edx
		mov byte ptr [eax], 0

		// Call a function in the runner that resets the projection
		push 0
		push 0
		push 0
		push 0
		mov eax, gmRenderWidth
		fild dword ptr [eax]
		add esp, -8
		fstp qword ptr [esp]
		wait
		mov eax, gmRenderHeight
		fild dword ptr [eax]
		add esp, -8
		fstp qword ptr [esp]
		wait
		push 0
		push 0
		mov edx, gmSetProjection
		call edx
		ret
	}
}

// The code for the hook that is called when a transition is done drawing.
// Again, no C++ functions are needed here.
__declspec(naked) void TransitionDoneHook()
{
	__asm
	{
		// Restore the value of the "perspective projection" variable
		mov edx, perspProj
		mov eax, gmPerspProj
		mov byte ptr [eax], dl

		// Execute the replaced code
		mov eax, gmWaitVSync
		call eax
		ret
	}
}

// This hook is for GM's DLL system, and it allows SurfaceToString (write_surface) to return a string with null characters in it.
// It also frees the string that SurfaceToString returns.
// Note: this is hax
__declspec(naked) void DLLHook()
{
	__asm
	{
		// If a null pointer was returned, let GM deal with it
		test edx, edx
		jz NormalCopy

		// If the returned pointer isn't tempString, let GM deal with it - it isn't SurfaceToString that was called
		cmp edx, tempString
		jne NormalCopy

		// Get the length of the string
		mov ecx, tempString
		mov ecx, [ecx]
		
		// Save registers
		push ebx
		push esi
		push edi

		// Allocate the string
		mov ebx, eax
		mov esi, edx
		mov edi, ecx
		mov eax, edi
		mov edx, gmAllocString
		call edx
		mov ecx, edi
		mov edi, eax

		// Copy the returned string data into the new string
		add ecx, 5
		push ecx
		push esi
		mov eax, edi
		sub eax, 4
		push eax
		call dword ptr [memcpy]
		add esp, 12

		// Free tempString
		mov eax, tempString
		push eax
		call dword ptr [free]
		add esp, 4
		mov tempString, 0

		// Clear out the old result string (if any)
		mov eax, ebx
		mov edx, gmClearString
		call edx

		// Set the pointer to the newly allocated string
		mov [ebx], edi

		pop edi
		pop esi
		pop ebx
		ret

NormalCopy:
		// Just call GM's set string function
		mov ecx, gmSetString
		jmp ecx
	}
}

// This is the hook for draw_clear() and draw_clear_alpha() that makes them clear the Z-buffer.
__declspec(naked) void DrawClearHook()
{
	__asm
	{
		// If we're not in 3D mode, just return
		mov eax, gmD3DMode
		mov eax, [eax]
		mov al, byte ptr [eax]
		test al, al
		jz Done

		// Set the flags for Clear()
		mov dword ptr [esp + 12], D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER

		// Set the Z-buffer value to 1.0f
		mov dword ptr [esp + 20], 0x3F800000

		// Execute the replaced code
Done:
		mov eax, gmD3DDevicePtr
		mov eax, [eax]
		ret
	}
}

// This hook checks the return value of d3dDevice->Present() to see if the device was lost.
// If the device was lost, it sets a variable (used by another hook) indicating that the game should be frozen until it can be reset.
__declspec(naked) void PresentSceneHook1()
{
	__asm
	{
		// Check if Present() failed due to a lost device
		cmp eax, 0x88760868	// D3DERR_DEVICELOST
		jne DeviceNotLost

		// Set the "device lost" variable
		mov deviceLost, 1

DeviceNotLost:
		xor eax, eax
		mov edx, gmPresentSceneHook1_Success
		jmp edx
	}
}

// This hook is similar to the above one except that it is for a different function.
__declspec(naked) void PresentSceneHook2()
{
	__asm
	{
		// Call Present() and check if the device was lost
		call dword ptr [eax + 0x3C]
		cmp eax, 0x88760868	// D3DERR_DEVICELOST
		jne DeviceNotLost

		// Set the "device lost" variable
		mov deviceLost, 1

DeviceNotLost:
		mov eax, 1
		mov edx, gmPresentSceneHook2_Success
		jmp edx
	}
}

// This is the game loop hook that tries to reset the device if it is lost.
__declspec(naked) void GameLoopHook()
{
	__asm
	{
		// Check the "device lost" variable
		mov eax, deviceLost
		test eax, eax
		jz DeviceNotLost

		// Call TestCooperativeLevel and see if the device can be reset
		mov eax, d3dDevice
		push eax
		mov eax, [eax]
		call dword ptr [eax + 0x0C]
		test eax, eax
		jz DeviceNotLost

		cmp eax, 0x88760869	// D3DERR_DEVICENOTRESET
		jne DeviceResetFailed

		// Try to reset the device
		mov edx, gmResetDevice
		call edx

		// Check whether or not it succeeded
		test eax, eax
		jnz DeviceNotLost

DeviceResetFailed:
		// Skip over the game loop
		mov eax, gmGameLoopHook_Failure
		mov dword ptr [esp], eax
		ret

DeviceNotLost:
		mov deviceLost, 0

		// Execute the replaced code and return
		mov edx, gmGameLoopHook_Success
		call edx
		ret
	}
}

// This detects the version of GM that is being used and sets pointers accordingly.
int DetectGMVersion()
{
	DWORD gmVersion = *gmVersionID;

	if (gmVersion == GM7_IP_ID)
	{
		gmSurfaceExists = GM7_IP_SURFACE_EXISTS;
		gmGetSurfacePtr = GM7_IP_GET_SURFACE_PTR;

		gmGetViewData = GM7_IP_GET_VIEW_DATA;

		gmSetProjection = GM7_IP_SET_PROJECTION;
		gmSetViewport = GM7_IP_SET_VIEWPORT;
		gmGetRenderSize = GM7_IP_GET_RENDER_SIZE;

		gmSetD3DStates = GM7_IP_SET_D3D_STATES;
		gmBeginScene = GM7_IP_BEGIN_SCENE;
		gmWaitVSync = GM7_IP_WAIT_VSYNC;

		gmResetTarget = GM7_IP_RESET_RENDER_TARGET;
		gmDestroySurfaces = GM7_IP_DESTROY_SURFACES;
		gmResetDevice = GM7_IP_RESET_DEVICE;

		gmCreateSurfaceHook = GM7_IP_CREATESURFACE_HOOK;
		gmCreateSurfaceHook_Success = GM7_IP_CREATESURFACE_HOOK_SUCCESS;
		gmCreateSurfaceHook_Error = GM7_IP_CREATESURFACE_HOOK_ERROR;

		gmSetRenderTargetHook = GM7_IP_SETRENDERTARGET_HOOK;
		gmSetRenderTargetHook_Success = GM7_IP_SETRENDERTARGET_HOOK_SUCCESS;

		gmFreeSurfaceHook = GM7_IP_FREESURFACE_HOOK;

		gmResetRenderTargetHook = GM7_IP_RESETRENDERTARGET_HOOK;
		gmResetRenderTargetHook_Success = GM7_IP_RESETRENDERTARGET_HOOK_SUCCESS;

		gmSetRenderTargetProjectionHook = GM7_IP_SETRENDERTARGET_PROJECTION_HOOK;
		gmSetRenderTargetProjectionHook_Success = GM7_IP_SETRENDERTARGET_PROJECTION_HOOK_SUCCESS;
		gmSetRenderTargetProjectionHook_NoViews = GM7_IP_SETRENDERTARGET_PROJECTION_HOOK_NOVIEWS;

		gmResetRenderTargetViewportHook = GM7_IP_RESETRENDERTARGET_VIEWPORT_HOOK;
		gmResetRenderTargetViewportHook_Success = GM7_IP_RESETRENDERTARGET_VIEWPORT_HOOK_SUCCESS;
		gmResetRenderTargetViewportHook_NoViews = GM7_IP_RESETRENDERTARGET_VIEWPORT_HOOK_NOVIEWS;

		gmBeforeDeviceResetHook = GM7_IP_BEFOREDEVICERESET_HOOK;
		gmAfterDeviceResetHook = GM7_IP_AFTERDEVICERESET_HOOK;

		gmTransitionBeginSceneHook = GM7_IP_TRANSITION_BEGINSCENE_HOOK;
		gmTransitionDoneHook = GM7_IP_TRANSITION_DONE_HOOK;

		gmD3DSwitchOverride1 = GM7_IP_D3DSWITCH_OVERRIDE_1;
		gmD3DSwitchOverride2 = GM7_IP_D3DSWITCH_OVERRIDE_2;

		gmDrawClearHook = GM7_IP_DRAWCLEAR_HOOK;
		gmDrawClearAlphaHook = GM7_IP_DRAWCLEARALPHA_HOOK;

		gmDLLHook = GM7_IP_DLLHOOK;

		gmPresentSceneHook1 = GM7_IP_PRESENTSCENE_HOOK_1;
		gmPresentSceneHook1_Success = GM7_IP_PRESENTSCENE_HOOK_1_SUCCESS;
		gmPresentSceneHook2 = GM7_IP_PRESENTSCENE_HOOK_2;
		gmPresentSceneHook2_Success = GM7_IP_PRESENTSCENE_HOOK_2_SUCCESS;

		gmGameLoopHook = GM7_IP_GAMELOOP_HOOK;
		gmGameLoopHook_Success = GM7_IP_GAMELOOP_HOOK_SUCCESS;
		gmGameLoopHook_Failure = GM7_IP_GAMELOOP_HOOK_FAILURE;
	}
	else if (gmVersion == GM8_ID)
	{
		// Set all of the pointers to the GM 8 ones
		gmSetString = GM8_SET_STRING;
		gmAllocString = GM8_ALLOCATE_STRING;
		gmClearString = GM8_CLEAR_STRING;

		gmBinFileStatus = (BYTE*)GM8_BINFILE_STATUS_ARRAY;
		gmBinFileData = (BYTE*)GM8_BINFILE_DATA_TABLE;

		gmSurfaceData = (BYTE**)GM8_SURFACE_DATA;
		gmCurrentSurface = (int*)GM8_CURRENT_SURFACE;
		gmSurfaceExists = GM8_SURFACE_EXISTS;
		gmGetSurfacePtr = GM8_GET_SURFACE_PTR;

		gmViewInfo = (BYTE*)GM8_VIEW_INFO;
		gmCurrentView = (DWORD**)GM8_VIEW_CURRENT;
		gmGetViewData = GM8_GET_VIEW_DATA;

		gmSetProjection = GM8_SET_PROJECTION;
		gmSetViewport = GM8_SET_VIEWPORT;
		gmGetRenderSize = GM8_GET_RENDER_SIZE;

		gmSetD3DStates = GM8_SET_D3D_STATES;
		gmBeginScene = GM8_BEGIN_SCENE;
		gmWaitVSync = GM8_WAIT_VSYNC;

		gmResetTarget = GM8_RESET_RENDER_TARGET;
		gmDestroySurfaces = GM8_DESTROY_SURFACES;
		gmResetDevice = GM8_RESET_DEVICE;

		gmD3DInterface = (LPDIRECT3D8*)GM8_D3D_INTERFACE;
		gmD3DDevice = (LPDIRECT3DDEVICE8*)GM8_D3D_DEVICE;
		gmD3DDevicePtr = (LPDIRECT3DDEVICE8**)GM8_D3D_DEVICE_PTR;
		gmScreenSurface = (LPDIRECT3DSURFACE8*)GM8_SCREEN_TARGET;
		gmRenderWidth = (DWORD*)GM8_RENDER_WIDTH;
		gmRenderHeight = (DWORD*)GM8_RENDER_HEIGHT;
		gmPerspProj = (BYTE*)GM8_PERSPECTIVE_PROJ;
		gmD3DMode = (BYTE*)GM8_D3D_MODE;
		gmDevCaps = (D3DCAPS8*)GM8_DEVICE_CAPS;

		gmCreateSurfaceHook = GM8_CREATESURFACE_HOOK;
		gmCreateSurfaceHook_Success = GM8_CREATESURFACE_HOOK_SUCCESS;
		gmCreateSurfaceHook_Error = GM8_CREATESURFACE_HOOK_ERROR;

		gmSetRenderTargetHook = GM8_SETRENDERTARGET_HOOK;
		gmSetRenderTargetHook_Success = GM8_SETRENDERTARGET_HOOK_SUCCESS;

		gmFreeSurfaceHook = GM8_FREESURFACE_HOOK;

		gmResetRenderTargetHook = GM8_RESETRENDERTARGET_HOOK;
		gmResetRenderTargetHook_Success = GM8_RESETRENDERTARGET_HOOK_SUCCESS;

		gmSetRenderTargetProjectionHook = GM8_SETRENDERTARGET_PROJECTION_HOOK;
		gmSetRenderTargetProjectionHook_Success = GM8_SETRENDERTARGET_PROJECTION_HOOK_SUCCESS;
		gmSetRenderTargetProjectionHook_NoViews = GM8_SETRENDERTARGET_PROJECTION_HOOK_NOVIEWS;

		gmResetRenderTargetViewportHook = GM8_RESETRENDERTARGET_VIEWPORT_HOOK;
		gmResetRenderTargetViewportHook_Success = GM8_RESETRENDERTARGET_VIEWPORT_HOOK_SUCCESS;
		gmResetRenderTargetViewportHook_NoViews = GM8_RESETRENDERTARGET_VIEWPORT_HOOK_NOVIEWS;

		gmBeforeDeviceResetHook = GM8_BEFOREDEVICERESET_HOOK;
		gmAfterDeviceResetHook = GM8_AFTERDEVICERESET_HOOK;

		gmTransitionBeginSceneHook = GM8_TRANSITION_BEGINSCENE_HOOK;
		gmTransitionDoneHook = GM8_TRANSITION_DONE_HOOK;

		gmD3DSwitchOverride1 = GM8_D3DSWITCH_OVERRIDE_1;
		gmD3DSwitchOverride2 = GM8_D3DSWITCH_OVERRIDE_2;

		gmDrawClearHook = GM8_DRAWCLEAR_HOOK;
		gmDrawClearAlphaHook = GM8_DRAWCLEARALPHA_HOOK;

		gmDLLHook = GM8_DLLHOOK;

		gmPresentSceneHook1 = GM8_PRESENTSCENE_HOOK_1;
		gmPresentSceneHook1_Success = GM8_PRESENTSCENE_HOOK_1_SUCCESS;
		gmPresentSceneHook2 = GM8_PRESENTSCENE_HOOK_2;
		gmPresentSceneHook2_Success = GM8_PRESENTSCENE_HOOK_2_SUCCESS;

		gmGameLoopHook = GM8_GAMELOOP_HOOK;
		gmGameLoopHook_Success = GM8_GAMELOOP_HOOK_SUCCESS;
		gmGameLoopHook_Failure = GM8_GAMELOOP_HOOK_FAILURE;
	}
	else if (gmVersion == GM8_IP_ID)
	{
		// Set all of the pointers to the GM 8 ones
		gmSetString = GM8_SET_STRING;
		gmAllocString = GM8_ALLOCATE_STRING;
		gmClearString = GM8_CLEAR_STRING;

		gmBinFileStatus = (BYTE*)GM8_BINFILE_STATUS_ARRAY;
		gmBinFileData = (BYTE*)GM8_BINFILE_DATA_TABLE;

		gmSurfaceData = (BYTE**)GM8_SURFACE_DATA;
		gmCurrentSurface = (int*)GM8_CURRENT_SURFACE;
		gmSurfaceExists = GM8_IP_SURFACE_EXISTS;
		gmGetSurfacePtr = GM8_IP_GET_SURFACE_PTR;

		gmViewInfo = (BYTE*)GM8_VIEW_INFO;
		gmCurrentView = (DWORD**)GM8_VIEW_CURRENT;
		gmGetViewData = GM8_IP_GET_VIEW_DATA;

		gmSetProjection = GM8_IP_SET_PROJECTION;
		gmSetViewport = GM8_IP_SET_VIEWPORT;
		gmGetRenderSize = GM8_IP_GET_RENDER_SIZE;

		gmSetD3DStates = GM8_IP_SET_D3D_STATES;
		gmBeginScene = GM8_IP_BEGIN_SCENE;
		gmWaitVSync = GM8_IP_WAIT_VSYNC;

		gmResetTarget = GM8_IP_RESET_RENDER_TARGET;
		gmDestroySurfaces = GM8_IP_DESTROY_SURFACES;
		gmResetDevice = GM8_IP_RESET_DEVICE;

		gmD3DInterface = (LPDIRECT3D8*)GM8_D3D_INTERFACE;
		gmD3DDevice = (LPDIRECT3DDEVICE8*)GM8_D3D_DEVICE;
		gmD3DDevicePtr = (LPDIRECT3DDEVICE8**)GM8_D3D_DEVICE_PTR;
		gmScreenSurface = (LPDIRECT3DSURFACE8*)GM8_SCREEN_TARGET;
		gmRenderWidth = (DWORD*)GM8_RENDER_WIDTH;
		gmRenderHeight = (DWORD*)GM8_RENDER_HEIGHT;
		gmPerspProj = (BYTE*)GM8_PERSPECTIVE_PROJ;
		gmD3DMode = (BYTE*)GM8_D3D_MODE;
		gmDevCaps = (D3DCAPS8*)GM8_DEVICE_CAPS;

		gmCreateSurfaceHook = GM8_IP_CREATESURFACE_HOOK;
		gmCreateSurfaceHook_Success = GM8_IP_CREATESURFACE_HOOK_SUCCESS;
		gmCreateSurfaceHook_Error = GM8_IP_CREATESURFACE_HOOK_ERROR;

		gmSetRenderTargetHook = GM8_IP_SETRENDERTARGET_HOOK;
		gmSetRenderTargetHook_Success = GM8_IP_SETRENDERTARGET_HOOK_SUCCESS;

		gmFreeSurfaceHook = GM8_IP_FREESURFACE_HOOK;

		gmResetRenderTargetHook = GM8_IP_RESETRENDERTARGET_HOOK;
		gmResetRenderTargetHook_Success = GM8_IP_RESETRENDERTARGET_HOOK_SUCCESS;

		gmSetRenderTargetProjectionHook = GM8_IP_SETRENDERTARGET_PROJECTION_HOOK;
		gmSetRenderTargetProjectionHook_Success = GM8_IP_SETRENDERTARGET_PROJECTION_HOOK_SUCCESS;
		gmSetRenderTargetProjectionHook_NoViews = GM8_IP_SETRENDERTARGET_PROJECTION_HOOK_NOVIEWS;

		gmResetRenderTargetViewportHook = GM8_IP_RESETRENDERTARGET_VIEWPORT_HOOK;
		gmResetRenderTargetViewportHook_Success = GM8_IP_RESETRENDERTARGET_VIEWPORT_HOOK_SUCCESS;
		gmResetRenderTargetViewportHook_NoViews = GM8_IP_RESETRENDERTARGET_VIEWPORT_HOOK_NOVIEWS;

		gmBeforeDeviceResetHook = GM8_IP_BEFOREDEVICERESET_HOOK;
		gmAfterDeviceResetHook = GM8_IP_AFTERDEVICERESET_HOOK;

		gmTransitionBeginSceneHook = GM8_IP_TRANSITION_BEGINSCENE_HOOK;
		gmTransitionDoneHook = GM8_IP_TRANSITION_DONE_HOOK;

		gmD3DSwitchOverride1 = GM8_IP_D3DSWITCH_OVERRIDE_1;
		gmD3DSwitchOverride2 = GM8_IP_D3DSWITCH_OVERRIDE_2;

		gmDrawClearHook = GM8_IP_DRAWCLEAR_HOOK;
		gmDrawClearAlphaHook = GM8_IP_DRAWCLEARALPHA_HOOK;

		gmDLLHook = GM8_IP_DLLHOOK;

		gmPresentSceneHook1 = GM8_IP_PRESENTSCENE_HOOK_1;
		gmPresentSceneHook1_Success = GM8_IP_PRESENTSCENE_HOOK_1_SUCCESS;
		gmPresentSceneHook2 = GM8_IP_PRESENTSCENE_HOOK_2;
		gmPresentSceneHook2_Success = GM8_IP_PRESENTSCENE_HOOK_2_SUCCESS;

		gmGameLoopHook = GM8_IP_GAMELOOP_HOOK;
		gmGameLoopHook_Success = GM8_IP_GAMELOOP_HOOK_SUCCESS;
		gmGameLoopHook_Failure = GM8_IP_GAMELOOP_HOOK_FAILURE;
	}
	else if (gmVersion != GM7_ID)
	{
		return 0;
	}

	return 1;
}

// Installs the runner hooks necessary to add surface support in 3D.
// It also retrieves some of the D3D pointers.
void InstallHooks()
{
	// Get the D3D interface and device pointers
	d3d = *gmD3DInterface;
	d3dDevice = *gmD3DDevice;

	// Get the pointer to the current depth buffer
	d3dDevice->GetDepthStencilSurface(&oldDepthBuffer);

	// Install a hook that is called when a surface is created
	InsertBranch(BRANCH_JUMP, (void*)gmCreateSurfaceHook, CreateSurfaceHookBase);

	// Install a hook that is called when the render target is set to a surface
	InsertBranch(BRANCH_JUMP, (void*)gmSetRenderTargetHook, SetRenderTargetHookBase);

	// Install a hook that is called when a surface is freed
	InsertBranch(BRANCH_CALL, (void*)gmFreeSurfaceHook, FreeSurfaceHookBase);

	// Install a hook that is called when the render target is reset
	InsertBranch(BRANCH_JUMP, (void*)gmResetRenderTargetHook, ResetRenderTargetHook);

	// Install the before/after device reset hooks
	InsertBranch(BRANCH_CALL, (void*)gmBeforeDeviceResetHook, BeforeDeviceResetHookBase);
	InsertBranch(BRANCH_CALL, (void*)gmAfterDeviceResetHook, AfterDeviceResetHookBase);

	// Install the transition hooks
	InsertBranch(BRANCH_CALL, (void*)gmTransitionBeginSceneHook, TransitionBeginSceneHook);
	InsertBranch(BRANCH_CALL, (void*)gmTransitionDoneHook, TransitionDoneHook);

	// Install the viewport/projection hooks
	InsertBranch(BRANCH_JUMP, (void*)gmSetRenderTargetProjectionHook, SetProjectionForSurfaceHook);
	InsertBranch(BRANCH_JUMP, (void*)gmResetRenderTargetViewportHook, ResetViewportHook);

	// Install the d3d_start/d3d_end overrides
	InsertBranch(BRANCH_CALL, (void*)gmD3DSwitchOverride1, D3DSwitchOverrideBase);
	InsertBranch(BRANCH_CALL, (void*)gmD3DSwitchOverride2, D3DSwitchOverrideBase);

	// Install the DLL hook
	InsertBranch(BRANCH_CALL, (void*)gmDLLHook, DLLHook);

	// Install the draw_clear() and draw_clear_alpha() hooks
	InsertBranch(BRANCH_CALL, (void*)gmDrawClearHook, DrawClearHook);
	InsertBranch(BRANCH_CALL, (void*)gmDrawClearAlphaHook, DrawClearHook);

	// Install the hooks that handle lost devices properly
	InsertBranch(BRANCH_JUMP, (void*)gmPresentSceneHook1, PresentSceneHook1);
	InsertBranch(BRANCH_JUMP, (void*)gmPresentSceneHook2, PresentSceneHook2);
	InsertBranch(BRANCH_CALL, (void*)gmGameLoopHook, GameLoopHook);
}