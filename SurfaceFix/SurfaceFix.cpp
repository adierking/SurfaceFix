// SurfaceFix.cpp - This is the main file. It includes the main hook code as well as the exported functions.

#include "stdafx.h"
#include "hooks.h"
#include "ascii85.h"

#define gm_export extern "C" __declspec(dllexport)

std::vector<LPDIRECT3DSURFACE8> depthBuffers;
BYTE* tempString = 0;

int surfaceFixEnabled = 1;
bool nextSurfaceIsDepthLockable = false;
D3DDEVICE_CREATION_PARAMETERS createParams;
D3DDISPLAYMODE displayMode;

// Round up an unsigned 32-bit integer to the nearest power of two.
unsigned int PowerOfTwo(unsigned int num)
{
	num--;
	num |= (num >> 1);
	num |= (num >> 2);
	num |= (num >> 4);
	num |= (num >> 8);
	num |= (num >> 16);
	num++;

	return num;
}

void UpdateDeviceInfo()
{
	d3dDevice->GetCreationParameters(&createParams);
	d3d->GetAdapterDisplayMode(createParams.AdapterOrdinal, &displayMode);
}

// Creates a new depth buffer and associates it with a surface.
int CreateSurfaceHook(int id, unsigned int width, unsigned int height)
{
	LPDIRECT3DSURFACE8 depthBuffer;

	// Check if the surface already has something associated with it
	// It looks like GM reuses surface ID's, so it's important to do this
	if (id < (int)depthBuffers.size())
	{
		if (depthBuffers[id])
		{
			depthBuffers[id]->Release();
			depthBuffers[id] = 0;
		}
	}

	// Round the width and height up to powers of two
	width = PowerOfTwo(width);
	height = PowerOfTwo(height);

	D3DFORMAT depthFormat;
	if (nextSurfaceIsDepthLockable)
	{
		nextSurfaceIsDepthLockable = false;

		// Make sure we can use D3DFMT_D16_LOCKABLE
		if (FAILED(d3d->CheckDeviceFormat(createParams.AdapterOrdinal, createParams.DeviceType, displayMode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16_LOCKABLE)))
			return 0;
		if (FAILED(d3d->CheckDepthStencilMatch(createParams.AdapterOrdinal, createParams.DeviceType, displayMode.Format, D3DFMT_A8R8G8B8, D3DFMT_D16_LOCKABLE)))
			return 0;

		depthFormat = D3DFMT_D16_LOCKABLE;
	}
	else
	{
		depthFormat = D3DFMT_D16;
	}

	// Create a new depth buffer
	if (FAILED(d3dDevice->CreateDepthStencilSurface(width, height, depthFormat, D3DMULTISAMPLE_NONE, &depthBuffer)))
	{
		return 0;
	}

	// Associate it with the surface
	if (id < (int)depthBuffers.size())
	{
		depthBuffers[id] = depthBuffer;
	}
	else
	{
		depthBuffers.push_back(depthBuffer);
	}
	return 1;
}

// Retrieves the depth buffer associated with the specified surface.
LPDIRECT3DSURFACE8 SetRenderTargetHook(int id)
{
	// Make sure the depth buffer exists
	if (id < 0 || id >= (int)depthBuffers.size())
	{
		return 0;
	}

	// Return the depth buffer pointer
	return depthBuffers[id];
}

// Frees the depth buffer associated with a surface.
void FreeSurfaceHook(int id)
{
	// If the depth buffer exists, free it
	if (id >= 0 && id < (int)depthBuffers.size())
	{
		if (depthBuffers[id]) depthBuffers[id]->Release();
		depthBuffers[id] = 0;
	}
}

// Frees the "old depth buffer" pointer
void BeforeDeviceResetHook()
{
	if (oldDepthBuffer)
	{
		oldDepthBuffer->Release();
		oldDepthBuffer->Release();
		oldDepthBuffer = 0;
	}
}

// Retrieves the new depth buffer pointer after a device reset.
void AfterDeviceResetHook()
{
	d3dDevice->GetDepthStencilSurface(&oldDepthBuffer);

	UpdateDeviceInfo();
}

// This is a replacement for the d3d_start and d3d_end functions.
// It doesn't replace the entire function, but it does replace a lot of it.
int D3DSwitchOverride(int switchToD3d)
{
	LPDIRECT3DSURFACE8 depthBuffer;
	unsigned int width;
	unsigned int height;
	D3DDISPLAYMODE displayMode;

	// Release the old depth-stencil buffer pointer
	if (oldDepthBuffer)
	{
		oldDepthBuffer->Release();
	}

	// Disassociate it with the device
	d3dDevice->SetRenderTarget(NULL, NULL);

	// Release the depth-stencil buffer
	if (oldDepthBuffer)
	{
		oldDepthBuffer->Release();
		oldDepthBuffer = 0;
	}

	// If we are switching to D3D mode, create a new depth-stencil buffer
	if (switchToD3d)
	{
		// Get the display mode to calculate surface size (it seems to be what GM does)
		d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
		width = PowerOfTwo(displayMode.Width);
		height = PowerOfTwo(displayMode.Height);

		// Create a new depth-stencil buffer
		if (FAILED(d3dDevice->CreateDepthStencilSurface(width, height, D3DFMT_D16, D3DMULTISAMPLE_NONE, &depthBuffer)))
		{
			return 0;
		}

		// Pair it with the color buffer
		if (FAILED(d3dDevice->SetRenderTarget(NULL, depthBuffer)))
		{
			depthBuffer->Release();
			return 0;
		}
	}

	// Get the new depth-stencil pointer
	d3dDevice->GetDepthStencilSurface(&oldDepthBuffer);

	// Get the device capabilities and store them to the runner's memory
	d3dDevice->GetDeviceCaps(gmDevCaps);

	// Set up some render states
	if (switchToD3d)
	{
		d3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}
	else
	{
		d3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	}

	// Clear the render target
	if (switchToD3d)
	{
		d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	}
	else
	{
		d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 0, 0);
	}

	return 1;
}

// Called when the DLL unloads, freeing all of the depth buffers
void Cleanup()
{
	std::vector<LPDIRECT3DSURFACE8>::iterator it;

	// Free all of the depth buffers
	for (it = depthBuffers.begin(); it != depthBuffers.end(); it++)
	{
		if (*it) (*it)->Release();
	}

	// Free the "old depth buffer" pointer
	if (oldDepthBuffer)
	{
		oldDepthBuffer->Release();
	}
}

// This returns whether or not a surface exists.
int SurfaceExists(int id)
{
	int exists = 0;

	__asm
	{
		// Call a runner function to check if it exists
		mov eax, id
		mov edx, gmSurfaceExists
		call edx
		test al, al
		jz DoesntExist
		mov exists, 1
DoesntExist:
	}

	return exists;
}


// This returns a pointer to an IDirect3DSurface8 interface, given a surface ID.
LPDIRECT3DSURFACE8 GetSurfaceInterface(int id)
{
	LPDIRECT3DSURFACE8 surface = 0;

	// Make sure the surface exists
	if (!SurfaceExists(id)) return 0;

	__asm
	{
		// Call a runner function to get the surface interface
		mov eax, gmSurfaceData
		mov eax, [eax]
		mov ecx, id
		add ecx, ecx
		mov eax, [eax + ecx * 8]
		lea edx, surface
		mov ecx, gmGetSurfacePtr
		call ecx
	}

	return surface;
}

// This retrieves a surface's width.
DWORD GetSurfaceWidth(int id)
{
	BYTE* surfaceData = *gmSurfaceData;

	if (id == -1)
	{
		// The screen
		return *gmRenderWidth;
	}
	else
	{
		return *(DWORD*)(surfaceData + id * 16 + 4);
	}
}

// This retrieves a surface's height.
DWORD GetSurfaceHeight(int id)
{
	BYTE* surfaceData = *gmSurfaceData;

	if (id == -1)
	{
		// The screen
		return *gmRenderHeight;
	}
	else
	{
		return *(DWORD*)(surfaceData + id * 16 + 8);
	}
}

// This returns a binary file's handle.
HANDLE GetBinaryFileHandle(int file, int write)
{
	// Check that the number is in the range 1...32
	if (file < 1 || file > 32)
	{
		return 0;
	}

	// Check that the file is open
	if (!gmBinFileStatus[file])
	{
		return 0;
	}

	// Check the file permissions
	if (write)
	{
		if (*(WORD*)(gmBinFileData + file * 0x53 * 4 + 4) == 0xD7B1)
		{
			return 0;
		}
	}
	else
	{
		if (*(WORD*)(gmBinFileData + file * 0x53 * 4 + 4) == 0xD7B2)
		{
			return 0;
		}
	}

	// Get the file's handle
	return (HANDLE)*(DWORD*)(gmBinFileData + file * 0x53 * 4);
}

// This function is supposed to be called from GM when the game initializes.
// It installs all of the runner hooks necessary to fix surface support.
gm_export double FixSurfaces()
{
	// Detect the version of GM that is being used
	if (!DetectGMVersion())
	{
		MessageBox(NULL, "Surface Fix has detected that you are using an unsupported version of Game Maker.\n\nIt has been disabled to prevent your game from crashing. Please switch to either Game Maker 7 or Game Maker 8 in order to use Surface Fix.\n\nNote that as of GameMaker 8.1.77, 3D surface support has been fixed in the runner and this extension is no longer needed.", "Surface Fix", MB_OK | MB_ICONWARNING);
		surfaceFixEnabled = 0;
		return 0;
	}

	// Install the hooks
	InstallHooks();
	return 1;
}

// This is the depth_clear() function.
gm_export double ClearDepthBuffer()
{
	if (!surfaceFixEnabled)
	{
		return 0;
	}

	// Clear the depth buffer
	d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	return 1;
}

// This is the depth_change() function.
gm_export double ChangeDepthBuffer(double id)
{
	D3DVIEWPORT8 viewport;
	int currentSurface;

	if (!surfaceFixEnabled)
	{
		return 0;
	}

	// Check that the depth buffer exists
	if (id < -1 || id >= (double)depthBuffers.size())
	{
		return 0;
	}

	currentSurface = *gmCurrentSurface;
	if (static_cast<int>(id) == currentSurface)
		return 1;

	if (GetSurfaceWidth((int)id) != GetSurfaceWidth(currentSurface) || GetSurfaceHeight((int)id) != GetSurfaceHeight(currentSurface))
	{
		return 0;
	}

	// Switch to it
	if (id == -1)
	{
		d3dDevice->SetRenderTarget(NULL, oldDepthBuffer);
	}
	else
	{
		d3dDevice->SetRenderTarget(NULL, depthBuffers[(int)id]);
	}

	// Set the viewport properly, because SetRenderTarget changes it
	viewport.X = 0;
	viewport.Y = 0;
	viewport.Width = GetSurfaceWidth(currentSurface);
	viewport.Height = GetSurfaceHeight(currentSurface);
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	d3dDevice->SetViewport(&viewport);

	return 1;
}

// This is the depth_write_enable() function.
gm_export double EnableDepthWriting(double enable)
{
	if (!surfaceFixEnabled)
	{
		return 0;
	}

	d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, enable != 0);
	return 1;
}

// This is the surface_get_target() function.
gm_export double GetCurrentSurface()
{
	if (!surfaceFixEnabled)
	{
		return -1;
	}

	// Get the current surface by reading GM's memory (it stores it somewhere)
	return (double)(*gmCurrentSurface);
}

// This is the surface_write() function.
gm_export char* SurfaceToString(double id, double format)
{
	LPDIRECT3DSURFACE8 surface = 0;
	LPDIRECT3DSURFACE8 imageSurface = 0;
	D3DLOCKED_RECT lockInfo;
	D3DSURFACE_DESC surfaceInfo;
	std::string asciiString;
	tempString = 0;

	if (!surfaceFixEnabled)
	{
		return 0;
	}

	// Make sure the format is valid
	if (format < 0 || format > 1)
	{
		return 0;
	}

	// Get the handle to the surface
	surface = GetSurfaceInterface((int)id);
	if (!surface)
	{
		return 0;
	}

	// D3DPOOL_DEFAULT textures can't be locked, so create an image surface and copy the surface to it
	surface->GetDesc(&surfaceInfo);
	if (FAILED(d3dDevice->CreateImageSurface(surfaceInfo.Width, surfaceInfo.Height, surfaceInfo.Format, &imageSurface)))
	{
		surface->Release();
		return 0;
	}
	if (FAILED(d3dDevice->CopyRects(surface, NULL, 0, imageSurface, NULL)))
	{
		imageSurface->Release();
		surface->Release();
		return 0;
	}

	// Lock the image surface
	if (FAILED(imageSurface->LockRect(&lockInfo, NULL, D3DLOCK_READONLY)))
	{
		imageSurface->Release();
		surface->Release();
		return 0;
	}

	// Create the buffer
	tempString = (BYTE*)malloc(18 + lockInfo.Pitch * surfaceInfo.Height);

	// Set the size of the string (for the DLL hook)
	*((DWORD*)tempString) = 13 + lockInfo.Pitch * surfaceInfo.Height;

	// Set the ID byte
	tempString[4] = 0xAA;

	// Copy in the surface width, height, and pitch
	*((DWORD*)&tempString[5]) = (DWORD)surfaceInfo.Width;
	*((DWORD*)&tempString[9]) = (DWORD)surfaceInfo.Height;
	*((DWORD*)&tempString[13]) = (DWORD)lockInfo.Pitch;

	// Copy the bytes in
	memcpy(&tempString[17], lockInfo.pBits, lockInfo.Pitch * surfaceInfo.Height);

	// If a text format was requested, encode the buffer
	if (format == 1)
	{
		asciiString = std::string("/") + Encode85((BYTE*)tempString + 4, 13 + lockInfo.Pitch * surfaceInfo.Height);
		
		// Replace the old buffer
		tempString = (BYTE*)realloc(tempString, 5 + asciiString.length());
		memset(tempString, 0, 5 + asciiString.length());
		*(DWORD*)tempString = (DWORD)asciiString.length();
		strcpy_s((char*)tempString + 4, asciiString.length() + 1, asciiString.c_str());
	}
	else
	{
		// Set the last byte to NULL
		tempString[17 + lockInfo.Pitch * surfaceInfo.Height] = 0;
	}

	// Unlock the surface, free it, and return
	imageSurface->UnlockRect();
	imageSurface->Release();
	surface->Release();
	return (char*)tempString;
}

// This is the surface_read() function.
gm_export double SurfaceFromString(double id, char* surfaceData)
{
	LPDIRECT3DSURFACE8 surface = 0;
	LPDIRECT3DSURFACE8 imageSurface = 0;
	D3DLOCKED_RECT lockInfo;
	D3DSURFACE_DESC surfaceInfo;
	DWORD strLength = 0;
	DWORD width = 0, height = 0, pitch = 0;
	const char* string = surfaceData;
	std::string decodedData;

	if (!surfaceFixEnabled)
	{
		return 0;
	}

	if (!string)
	{
		return 0;
	}

	// Get the length of the string
	strLength = *((DWORD*)(string - 4));

	// If the string is encoded, decode it
	if (surfaceData[0] == '/')
	{
		if (!Decode85(surfaceData + 1, strLength - 1, decodedData))
		{
			return 0;
		}
		string = decodedData.c_str();
		strLength = decodedData.length();
	}

	// Check the string length
	if (strLength < 13)
	{
		return 0;
	}

	// Check the first byte
	if ((unsigned char)string[0] != 0xAA)
	{
		return 0;
	}

	// Get the width, height, and pitch
	width = *((DWORD*)(string + 1));
	height = *((DWORD*)(string + 5));
	pitch = *((DWORD*)(string + 9));
	if (pitch < width * 4)
	{
		return 0;
	}
	if (strLength != 13 + pitch * height)
	{
		return 0;
	}

	// Get the handle to the surface
	surface = GetSurfaceInterface((int)id);
	if (!surface)
	{
		return 0;
	}

	// Check that the surface's width and height match the ones in the string
	surface->GetDesc(&surfaceInfo);
	if (surfaceInfo.Width != width || surfaceInfo.Height != height)
	{
		surface->Release();
		return 0;
	}

	// D3DPOOL_DEFAULT textures can't be locked, so create an image surface which the data will be written into
	if (FAILED(d3dDevice->CreateImageSurface(surfaceInfo.Width, surfaceInfo.Height, surfaceInfo.Format, &imageSurface)))
	{
		surface->Release();
		return 0;
	}

	// Lock the image surface
	if (FAILED(imageSurface->LockRect(&lockInfo, NULL, 0)))
	{
		imageSurface->Release();
		surface->Release();
		return 0;
	}
	if (lockInfo.Pitch != pitch)
	{
		imageSurface->UnlockRect();
		imageSurface->Release();
		surface->Release();
		return 0;
	}

	// Write the data in
	memcpy(lockInfo.pBits, string + 13, pitch * height);

	// Unlock the surface
	imageSurface->UnlockRect();

	// Copy the image surface to the destination surface
	d3dDevice->CopyRects(imageSurface, NULL, 0, surface, NULL);

	// Free the image surface and return
	imageSurface->Release();
	surface->Release();
	return 1;
}

// This is the file_bin_write_surface() function.
gm_export double WriteSurfaceToBinaryFile(double file, double id)
{
	char* surfaceData;
	HANDLE fileHandle;
	DWORD numWritten = 0;

	if (!surfaceFixEnabled)
	{
		return 0;
	}

	// Get the file handle
	fileHandle = GetBinaryFileHandle((int)file, 1);
	if (!fileHandle)
	{
		return 0;
	}

	// Get the surface's data
	surfaceData = SurfaceToString(id, 0);
	if (!surfaceData)
	{
		return 0;
	}

	// Write it to the file
	if (!WriteFile(fileHandle, surfaceData + 4, *(DWORD*)surfaceData, &numWritten, NULL))
	{
		free(surfaceData);
		surfaceData = 0;
		return 0;
	}
	if (numWritten != *(DWORD*)surfaceData)
	{
		free(surfaceData);
		surfaceData = 0;
		return 0;
	}

	free(surfaceData);
	surfaceData = 0;
	return 1;
}

// This is the file_bin_read_surface() function.
gm_export double ReadSurfaceFromBinaryFile(double file, double id)
{
	char* surfaceData;
	HANDLE fileHandle;
	DWORD numRead;
	DWORD height = 0, pitch = 0;

	if (!surfaceFixEnabled)
	{
		return 0;
	}

	// Get the file handle
	fileHandle = GetBinaryFileHandle((int)file, 0);
	if (!fileHandle)
	{
		return 0;
	}

	// Read the surface header in
	surfaceData = (char*)malloc(18);
	if (!ReadFile(fileHandle, surfaceData + 4, 13, &numRead, NULL))
	{
		free(surfaceData);
		return 0;
	}
	if (numRead != 13)
	{
		free(surfaceData);
		return 0;
	}

	// Grab the height and pitch
	height = *((DWORD*)(surfaceData + 9));
	pitch = *((DWORD*)(surfaceData + 13));

	// Resize the buffer
	surfaceData = (char*)realloc(surfaceData, 18 + pitch * height);
	if (!surfaceData)
	{
		return 0;
	}

	// Read in the rest of the data
	if (!ReadFile(fileHandle, surfaceData + 17, pitch * height, &numRead, NULL))
	{
		free(surfaceData);
		return 0;
	}
	if (numRead != pitch * height)
	{
		free(surfaceData);
		return 0;
	}

	// Set up the string
	*(DWORD*)surfaceData = 13 + numRead;
	surfaceData[17 + numRead] = 0;

	// Try to build the surface from it
	if (!SurfaceFromString(id, surfaceData + 4))
	{
		free(surfaceData);
		return 0;
	}

	free(surfaceData);
	return 1;
}

// Used internally by surface_create_depth_lockable
gm_export double SetDepthBufferLockable()
{
	nextSurfaceIsDepthLockable = true;
	return 0;
}