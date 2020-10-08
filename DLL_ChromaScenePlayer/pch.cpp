// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"
#include <Windows.h>
typedef unsigned char byte;
#include "Razer\ChromaAnimationAPI.h"
#include <mutex>

using namespace ChromaSDK;
using namespace std;

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

static bool _sInitializedAPI = false;
static bool _sChromaInitialized = false;
static bool _sWaitForExit = true;
static mutex _sMutex;

extern "C"
{
	__declspec(dllexport) int ApplicationStart()
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		if (!_sInitializedAPI)
		{
			if (ChromaAnimationAPI::InitAPI() != 0)
			{
				return -1;
			}
			_sInitializedAPI = true;
		}

		// start the main thread worker

		return 0;
	}

	__declspec(dllexport) int ApplicationQuit()
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		_sWaitForExit = false;
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

	__declspec(dllexport) long PlayerChromaInit()
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

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
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

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
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		return 0;
	}
	__declspec(dllexport) int PlayerSelectScene(int sceneIndex)
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		return 0;
	}	
}
