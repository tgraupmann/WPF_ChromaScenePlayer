// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

EXPORT_API LONG PlayerChromaInit()
{
	return 0;
}
EXPORT_API LONG PlayerChromaUninit()
{
	return 0;
}
EXPORT_API int PlayerLoadScene(const char* path)
{
	return 0;
}
EXPORT_API int PlayerSelectScene(int sceneIndex)
{
	return 0;
}
EXPORT_API int PlayerQuit()
{
	return 0;
}
