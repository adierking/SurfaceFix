// dllmain.cpp - The entry point is in here!

#include "stdafx.h"

extern void Cleanup();

BOOL APIENTRY DllMain(HMODULE module, DWORD callReason, LPVOID reserved)
{
	switch (callReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(module);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Cleanup();
		break;
	}
	return TRUE;
}
