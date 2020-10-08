// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

extern "C"
{
	__declspec(dllexport) long PlayerChromaInit()
	{
		return 0;
	}
	__declspec(dllexport) long PlayerChromaUninit()
	{
		return 0;
	}
	__declspec(dllexport) int PlayerLoadScene(const char* path)
	{
		return 0;
	}
	__declspec(dllexport) int PlayerSelectScene(int sceneIndex)
	{
		return 0;
	}
	__declspec(dllexport) int PlayerQuit()
	{
		return 0;
	}
}
