// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"
#include <Windows.h>
typedef unsigned char byte;
#include "Razer\ChromaAnimationAPI.h"

using namespace ChromaSDK;
using namespace std;

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

static bool _sInitializedAPI = false;
static bool _sChromaInitialized = false;

extern "C"
{
	__declspec(dllexport) long PlayerChromaInit()
	{
		if (!_sInitializedAPI)
		{
			if (ChromaAnimationAPI::InitAPI() != 0)
			{
				return -1;
			}
			_sInitializedAPI = true;
		}
		if (_sChromaInitialized)
		{
			return 0;
		}
		RZRESULT result = ChromaAnimationAPI::Init();
		if (result == 0)
		{
			_sChromaInitialized = true;
		}
		return result;
	}
	__declspec(dllexport) long PlayerChromaUninit()
	{
		if (_sInitializedAPI)
		{
			if (_sChromaInitialized)
			{
				ChromaAnimationAPI::StopAll();
				ChromaAnimationAPI::CloseAll();
				RZRESULT result = ChromaAnimationAPI::Uninit();
				if (result == 0)
				{
					_sChromaInitialized = false;
				}
				return result;
			}
		}
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
		if (_sInitializedAPI)
		{
			if (ChromaAnimationAPI::IsInitialized())
			{
				ChromaAnimationAPI::StopAll();
				ChromaAnimationAPI::CloseAll();
				return ChromaAnimationAPI::Uninit();
			}
		}
		return 0;
	}
}
